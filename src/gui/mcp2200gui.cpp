/*
Copyright (c) 2016-2018, Albertas Vyšniauskas
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "mcp2200gui.h"
#include "enum.h"
#include "mcp2200.h"
#include "paths.h"
#include "types.h"
#include "udev/udev.h"
#include <gtk/gtk.h>
#include <json/json.h>
#include <hidapi/hidapi.h>
#include <cstddef>
#include <thread>
#include <mutex>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
namespace fs = std::filesystem;
namespace gui
{
	enum class DeviceColumn: int
	{
		path = 0,
		serial = 1,
		manufacturer = 2,
		product = 3,
		release_number = 4,
	};
	struct Program::Impl
	{
		Program *m_decl;
		GtkWidget *m_window, *m_menu, *m_notebook, *m_device_list;
		GtkWidget *m_invert, *m_flow_control, *m_usb_configure, *m_suspend, *m_fast_blink;
		GtkWidget *m_tx[3], *m_rx[3];
		struct Gpio
		{
			GtkWidget *value, *input, *output, *default_value;
		};
		Gpio m_gpio[8];
		GtkWidget *m_vid, *m_pid, *m_product, *m_manufacturer;
		GtkWidget *m_new_vid, *m_new_pid, *m_new_product, *m_new_manufacturer;
		GtkWidget *m_description_scrolled_widget, *m_device_scrolled_widget;
		GDBusObjectManager *m_manager;
		thread m_usb_event_thread;
		string m_current_device;
		struct Configuration
		{
			uint16_t vid, pid;
			string manufacturer, product;
		}m_configuration;
		Json::Value m_configuration_json;
		mapping::Layout m_configuration_layout;
		bool m_closing;
		mutex m_before_close_mutex;
		function<void()> m_before_close;
		Impl(Program *decl):
			m_decl(decl),
			m_manager(nullptr),
			m_configuration_layout("Configuration"),
			m_closing(false)
		{
			m_configuration_layout
				(new Uint16HexValue("vid", offsetof(Configuration, vid), mcp2200::defaultVendorId))
				(new Uint16HexValue("pid", offsetof(Configuration, pid), mcp2200::defaultProductId))
				(new StringValue("manufacturer", offsetof(Configuration, manufacturer), ""))
				(new StringValue("product", offsetof(Configuration, product), ""));
			loadConfiguration();
		}
		fs::path getConfigurationPath()
		{
			auto config_path = paths::getConfigurationPath() / "mcp2200gui";
			std::error_code ec;
			if (!fs::exists(config_path, ec)){
				fs::create_directory(config_path, ec);
			}
			return config_path;
		}
		bool loadConfiguration()
		{
			using namespace mapping;
			ReaderWriter<Json::Value> reader;
			reader
				(new Uint16HexAdapter())
				(new StringAdapter());
			fs::path config_path;
			try{
				config_path = getConfigurationPath() / "configuration.json";
			}catch(const exception &e){
				reader.load(m_configuration_layout, m_configuration_json, &m_configuration);
				return false;
			}
			ifstream file(config_path.string(), ifstream::binary);
			if (file.is_open()){
				try {
					file >> m_configuration_json;
				} catch (const std::exception &e) {
					m_configuration_json = Json::Value(Json::objectValue);
				}
				file.close();
			}
			reader.load(m_configuration_layout, m_configuration_json, &m_configuration);
			return true;
		}
		bool saveConfiguration()
		{
			using namespace mapping;
			fs::path config_path;
			try{
				config_path = getConfigurationPath() / "configuration.json";
			}catch(const exception &e){
				return false;
			}
			ReaderWriter<Json::Value> writer;
			writer
				(new Uint16HexAdapter())
				(new StringAdapter());
			writer.save(m_configuration_layout, &m_configuration, m_configuration_json);
			ofstream file(config_path.string());
			if (file.is_open()){
				file << m_configuration_json;
				file.close();
			}
			return true;
		}
		void beforeDestroy()
		{
			{
				lock_guard<mutex> lock(m_before_close_mutex);
				m_closing = true;
				if (m_before_close)
					m_before_close();
			}
			m_usb_event_thread.join();
			saveConfiguration();
		}
		static void onDestroy(GtkWidget *, Impl *app)
		{
			app->beforeDestroy();
			gtk_main_quit();
		}
		static GtkWidget* newMenuItem(const char *label, const char *icon_name)
		{
			GtkWidget* menu_item = gtk_menu_item_new_with_mnemonic(label);
			gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_MENU);
			return menu_item;
		}
		void createMainMenu()
		{
			GtkMenu *menu;
			GtkWidget *item;
			GtkWidget* file_item;
			m_menu = gtk_menu_bar_new();
			menu = GTK_MENU(gtk_menu_new());
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
			item = newMenuItem("_Quit", "action-quit");
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
			g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(onDestroy), this);
			file_item = gtk_menu_item_new_with_mnemonic("_File");
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), GTK_WIDGET(menu));
			gtk_menu_shell_append(GTK_MENU_SHELL(m_menu), file_item);
			menu = GTK_MENU(gtk_menu_new());
		}
		void setMargins(GtkWidget *widget)
		{
			gtk_widget_set_margin_start(widget, 5);
			gtk_widget_set_margin_end(widget, 5);
			gtk_widget_set_margin_top(widget, 5);
			gtk_widget_set_margin_bottom(widget, 5);
		}
		void addColumn(GtkWidget *list, GtkListStore *, const char *title, DeviceColumn column)
		{
			GtkTreeViewColumn *col = gtk_tree_view_column_new();
			gtk_tree_view_column_set_title(col, title);
			gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
			GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
			gtk_tree_view_column_pack_start(col, renderer, true);
			gtk_tree_view_column_add_attribute(col, renderer, "text", enum_value(column));
			g_object_set(renderer, "editable", false, nullptr);
			gtk_tree_view_append_column(GTK_TREE_VIEW(list), col);
		}
		GtkWidget* createScrolledWindow()
		{
			GtkWidget *scrolled_window = gtk_scrolled_window_new(0, 0);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_IN);
			gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_window), 100);
			return scrolled_window;
		}
		GtkWidget* createDeviceList(GtkWidget **device_list)
		{
			GtkWidget *list = gtk_tree_view_new();
			if (device_list){
				*device_list = list;
			}
			gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), true);
			g_signal_connect(G_OBJECT(list), "cursor-changed", G_CALLBACK(onCursorChanged), this);
			g_signal_connect(G_OBJECT(list), "row-activated", G_CALLBACK(onRowActivated), this);
			GtkListStore *store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
			addColumn(list, store, "Path", DeviceColumn::path);
			addColumn(list, store, "Serial", DeviceColumn::serial);
			addColumn(list, store, "Manufacturer", DeviceColumn::manufacturer);
			addColumn(list, store, "Product", DeviceColumn::product);
			addColumn(list, store, "Release number", DeviceColumn::release_number);

			gtk_tree_view_set_enable_search(GTK_TREE_VIEW(list), false);
			gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
			g_object_unref(GTK_TREE_MODEL(store));
			GtkWidget *scrolled_window = createScrolledWindow();
			gtk_container_add(GTK_CONTAINER(scrolled_window), list);
			return scrolled_window;
		}
		void addDevice(const mcp2200::DeviceInformation &device)
		{
			auto model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_device_list));
			GtkTreeIter iter;
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				enum_value(DeviceColumn::path), device.path.c_str(),
				enum_value(DeviceColumn::serial), device.serial.c_str(),
				enum_value(DeviceColumn::manufacturer), device.manufacturer.c_str(),
				enum_value(DeviceColumn::product), device.product.c_str(),
				enum_value(DeviceColumn::release_number), device.release_number,
				-1);
		}
		void clearDevices()
		{
			auto model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_device_list));
			gtk_list_store_clear(GTK_LIST_STORE(model));
		}
		void showDevices()
		{
			mcp2200::Device device;
			if (device.find(m_configuration.vid, m_configuration.pid)){
				clearDevices();
				for (size_t i = 0; i < device.getCount(); i++){
					addDevice(device[i]);
				}
			}else{
				clearDevices();
			}
		}
		void setCurrentState(const mcp2200::Command &state)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_invert), state.getInvert());
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_usb_configure), state.getUsbConfigure());
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_flow_control), state.getFlowControl());
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_fast_blink), !state.getBlinkSpeed());
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_suspend), state.getSuspend());
			switch (state.getRxLedMode()){
				case mcp2200::LedMode::off:
				case mcp2200::LedMode::on:
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_rx[0]), true);
					break;
				case mcp2200::LedMode::blink:
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_rx[1]), true);
					break;
				case mcp2200::LedMode::toggle:
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_rx[2]), true);
					break;
			}
			switch (state.getTxLedMode()){
				case mcp2200::LedMode::off:
				case mcp2200::LedMode::on:
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_tx[0]), true);
					break;
				case mcp2200::LedMode::blink:
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_tx[1]), true);
					break;
				case mcp2200::LedMode::toggle:
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_tx[2]), true);
					break;
			}
			uint8_t gpio = state.getGpioValues();
			uint8_t gpio_directions = state.getIoDirections();
			uint8_t gpio_defaults = state.getDefaultValues();
			uint8_t gpio_mask = state.getIoMask();
			for (int i = 0; i < 8; i++){
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_gpio[i].value), ((gpio >> i) & 1) == 1);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_gpio[i].input), ((gpio_directions >> i) & 1) == 1);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_gpio[i].output), ((gpio_directions >> i) & 1) != 1);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_gpio[i].default_value), ((gpio_defaults >> i) & 1) == 1);
				gtk_widget_set_sensitive(m_gpio[i].value, ((gpio_mask >> i) & 1) == 1);
				gtk_widget_set_sensitive(m_gpio[i].input, ((gpio_mask >> i) & 1) == 1);
				gtk_widget_set_sensitive(m_gpio[i].output, ((gpio_mask >> i) & 1) == 1);
				gtk_widget_set_sensitive(m_gpio[i].default_value, ((gpio_mask >> i) & 1) == 1);
			}
		}
		bool openDevice(const char *device_path = nullptr)
		{
			if (device_path){
				m_current_device = device_path;
			}
			mcp2200::Device device;
			if (!device.open(m_current_device.c_str())){
				return false;
			}
			std::string manufacturer, product;
			device.getManufacturer(manufacturer);
			device.getProduct(product);
			string vid = toHexString(m_configuration.vid);
			string pid = toHexString(m_configuration.pid);
			gtk_entry_set_text(GTK_ENTRY(m_new_pid), pid.c_str());
			gtk_entry_set_text(GTK_ENTRY(m_new_vid), vid.c_str());
			gtk_entry_set_text(GTK_ENTRY(m_new_manufacturer), manufacturer.c_str());
			gtk_entry_set_text(GTK_ENTRY(m_new_product), product.c_str());
			mcp2200::Command response;
			if (!device.readAll(response)){
				return false;
			}
			setCurrentState(response);
			return true;
		}
		mcp2200::LedMode getLedMode(GtkWidget **widgets)
		{
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets[1])))
				return mcp2200::LedMode::blink;
			else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets[2])))
				return mcp2200::LedMode::toggle;
			else
				return mcp2200::LedMode::off;
		}
		void toCommand(mcp2200::Command &command)
		{
			using namespace mcp2200;
			command.setCommand(CommandType::configure);
			command
				.setSuspend(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_suspend)))
				.setUsbConfigure(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_usb_configure)))
				.setInvert(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_invert)))
				.setFlowControl(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_flow_control)))
				.setBlinkSpeed(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_fast_blink)));
			auto rx_mode = getLedMode(m_rx);
			if (rx_mode != LedMode::off)
				command.setRxLedMode(rx_mode);
			auto tx_mode = getLedMode(m_tx);
			if (tx_mode != LedMode::off)
				command.setTxLedMode(tx_mode);
			uint8_t gpio_mask = command.getIoMask();
			uint8_t gpio = 0, gpio_directions = 0, gpio_defaults = 0;
			for (int i = 0; i < 8; i++){
				gpio |= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_gpio[i].value)) << i;
				gpio_directions |= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_gpio[i].input)) << i;
				gpio_defaults |= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_gpio[i].default_value)) << i;
			}
			command
				.setGpioValues(gpio & gpio_mask, ~gpio & gpio_mask)
				.setDefaultValues((gpio_defaults & gpio_mask) | (command.getDefaultValues() & ~gpio_mask))
				.setIoDirections((gpio_directions & gpio_mask) | (command.getIoDirections() & ~gpio_mask));
		}
		void toGpioValuesCommand(mcp2200::Command &command)
		{
			using namespace mcp2200;
			uint8_t gpio_mask = command.getIoMask();
			uint8_t gpio = 0;
			for (int i = 0; i < 8; i++){
				gpio |= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_gpio[i].value)) << i;
			}
			command
				.setCommand(CommandType::set_clear_outputs)
				.setGpioValues(gpio & gpio_mask, ~gpio & gpio_mask);
		}
		void getGpioMask()
		{
			using namespace mcp2200;
			Command command;
			toCommand(command);
			uint8_t gpio_mask = command.getIoMask();
			for (int i = 0; i < 8; i++){
				gtk_widget_set_sensitive(m_gpio[i].value, ((gpio_mask >> i) & 1) == 1);
				gtk_widget_set_sensitive(m_gpio[i].input, ((gpio_mask >> i) & 1) == 1);
				gtk_widget_set_sensitive(m_gpio[i].output, ((gpio_mask >> i) & 1) == 1);
				gtk_widget_set_sensitive(m_gpio[i].default_value, ((gpio_mask >> i) & 1) == 1);
			}
		}
		bool openCurrentDevicePath(mcp2200::Device &device)
		{
			string current_device_path;
			GtkTreePath *path;
			gtk_tree_view_get_cursor(GTK_TREE_VIEW(m_device_list), &path, nullptr);
			if (path != nullptr){
				GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_device_list));
				GtkTreeIter iter;
				gtk_tree_model_get_iter(model, &iter, path);
				gchar *device_path = nullptr;
				gtk_tree_model_get(model, &iter, enum_value(DeviceColumn::path), &device_path, -1);
				if (device_path){
					current_device_path = device_path;
					g_free(device_path);
				}else{
					gtk_tree_path_free(path);
					return false;
				}
				gtk_tree_path_free(path);
			}
			if (!device.open(current_device_path.c_str())){
				return false;
			}
			return true;
		}
		bool applyToDevice()
		{
			using namespace mcp2200;
			Device device;
			if (!openCurrentDevicePath(device)){
				return false;
			}
			Command response;
			if (!device.readAll(response)){
				return false;
			}
			Command command(response);
			toCommand(command);
			if (!device.write(command)){
				return false;
			}
			if (!device.readAll(response)){
				return false;
			}
			setCurrentState(response);
			return true;
		}
		bool applyGpioValuesToDevice()
		{
			using namespace mcp2200;
			Device device;
			if (!openCurrentDevicePath(device)){
				return false;
			}
			Command response;
			if (!device.readAll(response)){
				return false;
			}
			Command command(response);
			toGpioValuesCommand(command);
			if (!device.write(command)){
				return false;
			}
			if (!device.readAll(response)){
				return false;
			}
			setCurrentState(response);
			return true;
		}
		void addCheckbox(GtkWidget *grid, int position, int column, const char *label, GtkWidget **checkbox, bool expand = true, bool center = false, void(*toggle)(GtkWidget*, Impl*) = nullptr)
		{
			GtkWidget *widget = gtk_check_button_new_with_label(label);
			if (expand)
				gtk_widget_set_hexpand(widget, true);
			if (center)
				gtk_widget_set_halign(widget, GTK_ALIGN_CENTER);
			gtk_grid_attach(GTK_GRID(grid), widget, column * 2, position, 2, 1);
			g_signal_connect(GTK_TOGGLE_BUTTON(widget), "toggled", G_CALLBACK(toggle), this);
			if (checkbox)
				*checkbox = widget;
		}
		void addToggle(GtkWidget *grid, int position, int column, const char *label_off, const char *label_on, GtkWidget **button_off, GtkWidget **button_on, bool expand = true, void(*toggle)(GtkWidget*, Impl*) = nullptr)
		{
			GSList *group = nullptr;
			GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			GtkWidget *widget = gtk_radio_button_new_with_label(group, label_off);
			g_signal_connect(GTK_TOGGLE_BUTTON(widget), "toggled", G_CALLBACK(toggle), this);
			if (button_off)
				*button_off = widget;
			group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
			gtk_box_pack_start(GTK_BOX(hbox), widget, false, false, 0);
			widget = gtk_radio_button_new_with_label(group, label_on);
			g_signal_connect(GTK_TOGGLE_BUTTON(widget), "toggled", G_CALLBACK(toggle), this);
			gtk_box_pack_start(GTK_BOX(hbox), widget, false, false, 0);
			if (expand)
				gtk_widget_set_hexpand(hbox, true);
			gtk_grid_attach(GTK_GRID(grid), hbox, column * 2, position, 2, 1);
			if (button_on)
				*button_on = widget;
		}
		void addOption(GtkWidget *grid, int position, int column, const char *label, GSList **group, GtkWidget **option, bool expand = true, void(*toggle)(GtkWidget*, Impl*) = nullptr)
		{
			GtkWidget *widget = gtk_radio_button_new_with_label(group != nullptr ? *group : nullptr, label);
			g_signal_connect(GTK_TOGGLE_BUTTON(widget), "toggled", G_CALLBACK(toggle), this);
			if (expand)
				gtk_widget_set_hexpand(widget, true);
			gtk_grid_attach(GTK_GRID(grid), widget, column * 2, position, 2, 1);
			if (option)
				*option = widget;
			if (group){
				*group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
			}
		}
		void addLabel(GtkWidget *grid, int position, int column, const char *label, GtkWidget **label_widget = nullptr, bool expand = true, bool center = false)
		{
			GtkWidget *widget = gtk_label_new(label);
			gtk_label_set_xalign(GTK_LABEL(widget), 0.0f);
			if (expand)
				gtk_widget_set_hexpand(widget, true);
			if (center)
				gtk_widget_set_halign(widget, GTK_ALIGN_CENTER);
			gtk_grid_attach(GTK_GRID(grid), widget, column * 2, position, 2, 1);
			if (label_widget)
				*label_widget = widget;
		}
		void addEntry(GtkWidget *grid, int position, int column, const char *value, GtkWidget **entry_widget = nullptr, bool expand = true, bool center = false)
		{
			GtkWidget *widget = gtk_entry_new();
			if (value)
				gtk_entry_set_text(GTK_ENTRY(widget), value);
			if (expand)
				gtk_widget_set_hexpand(widget, true);
			if (center)
				gtk_widget_set_halign(widget, GTK_ALIGN_CENTER);
			gtk_grid_attach(GTK_GRID(grid), widget, column * 2, position, 2, 1);
			if (entry_widget)
				*entry_widget = widget;
		}
		string toHexString(uint16_t value)
		{
			stringstream s;
			s << setfill('0') << hex << setw(4) << value;
			return s.str();
		}
		uint16_t toUint16(const string &value)
		{
			stringstream s(value);
			uint16_t result;
			s >> hex >> result;
			return result;
		}
		GtkWidget *addPageWithGrid(const char *label)
		{
			GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
			setMargins(vbox);
			gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), vbox, gtk_label_new(label));
			GtkWidget *grid = gtk_grid_new();
			gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
			gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
			gtk_box_pack_start(GTK_BOX(vbox), grid, false, true, 10);
			return grid;
		}
		int run(int, char **)
		{
			gtk_init(nullptr, nullptr);
			m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
			gtk_window_set_title(GTK_WINDOW(m_window), "MCP2200 Configuration");
			g_signal_connect(G_OBJECT(m_window), "delete_event", G_CALLBACK(onDeleteEvent), this);
			g_signal_connect(G_OBJECT(m_window), "destroy", G_CALLBACK(onDestroy), this);
			gtk_window_resize(GTK_WINDOW(m_window), 640, 540);
			GtkWidget* vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
			createMainMenu();
			gtk_box_pack_start(GTK_BOX(vbox_main), m_menu, false, false, 0);
			m_notebook = gtk_notebook_new();

			GtkWidget *grid = addPageWithGrid("Properties");
			GtkWidget *vbox = gtk_widget_get_parent(grid);
			gtk_box_pack_start(GTK_BOX(vbox), m_device_scrolled_widget = createDeviceList(&m_device_list), true, true, 0);
			gtk_box_reorder_child(GTK_BOX(vbox), grid, 1);

			addLabel(grid, 0, 0, "Options");
			addCheckbox(grid, 1, 0, "Invert", &m_invert, true, false, onChange);
			addCheckbox(grid, 2, 0, "Suspend", &m_suspend, true, false, onChange);
			addCheckbox(grid, 3, 0, "Flow control", &m_flow_control, true, false, onChange);
			addCheckbox(grid, 4, 0, "Configure", &m_usb_configure, true, false, onChange);
			addCheckbox(grid, 5, 0, "Fast blink", &m_fast_blink, true, false, onChange);

			addLabel(grid, 0, 1, "Receive LED");
			GSList *group = nullptr;
			addOption(grid, 1, 1, "None", &group, &m_rx[0], true, onChange);
			addOption(grid, 2, 1, "Blink", &group, &m_rx[1], true, onChange);
			addOption(grid, 3, 1, "Toggle", &group, &m_rx[2], true, onChange);

			addLabel(grid, 0, 2, "Transmit LED");
			group = nullptr;
			addOption(grid, 1, 2, "None", &group, &m_tx[0], true, onChange);
			addOption(grid, 2, 2, "Blink", &group, &m_tx[1], true, onChange);
			addOption(grid, 3, 2, "Toggle", &group, &m_tx[2], true, onChange);

			addLabel(grid, 0, 4, "Direction", nullptr, false, true);
			for (int i = 0; i < 8; i++){
				char label[6] = {"GPIO0"};
				label[4] = '0' + i;
				addLabel(grid, i + 1, 3, label, nullptr, false);
				addToggle(grid, i + 1, 4, "Input", "Output", &m_gpio[i].input, &m_gpio[i].output, false, onChange);
			}

			addLabel(grid, 0, 5, "Default", nullptr, false, true);
			for (int i = 0; i < 8; i++){
				addCheckbox(grid, i + 1, 5, "", &m_gpio[i].default_value, false, true, onChange);
			}

			addLabel(grid, 0, 6, "Value", nullptr, false, true);
			for (int i = 0; i < 8; i++){
				addCheckbox(grid, i + 1, 6, "", &m_gpio[i].value, false, true, onChange);
			}

			GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
			gtk_box_pack_start(GTK_BOX(vbox), hbox, false, true, 0);
			gtk_widget_set_halign(hbox, GTK_ALIGN_END);

			GtkWidget *apply = gtk_button_new_with_label("Apply");
			g_signal_connect(apply, "clicked", G_CALLBACK(onApply), this);
			gtk_box_pack_start(GTK_BOX(hbox), apply, false, false, 0);
			GtkWidget *apply_gpio_values = gtk_button_new_with_label("Apply GPIO values");
			g_signal_connect(apply_gpio_values, "clicked", G_CALLBACK(onApplyGpioValues), this);
			gtk_box_pack_start(GTK_BOX(hbox), apply_gpio_values, false, false, 0);
			GtkWidget *reload = gtk_button_new_with_label("Reload");
			g_signal_connect(reload, "clicked", G_CALLBACK(onReload), this);
			gtk_box_pack_start(GTK_BOX(hbox), reload, false, false, 0);
			GtkWidget *refresh = gtk_button_new_with_label("Refresh device list");
			g_signal_connect(refresh, "clicked", G_CALLBACK(onRefresh), this);
			gtk_box_pack_start(GTK_BOX(hbox), refresh, false, false, 0);

			grid = addPageWithGrid("Description");
			gtk_widget_set_vexpand(grid, false);
			vbox = gtk_widget_get_parent(grid);
			gtk_box_pack_start(GTK_BOX(vbox), m_description_scrolled_widget = createScrolledWindow(), true, true, 0);
			gtk_box_reorder_child(GTK_BOX(vbox), grid, 1);

			addLabel(grid, 0, 0, "Vendor ID", nullptr, false);
			addEntry(grid, 0, 1, "", &m_new_vid, true);
			addLabel(grid, 1, 0, "Product ID", nullptr, false);
			addEntry(grid, 1, 1, "", &m_new_pid, true);
			addLabel(grid, 2, 0, "Product", nullptr, false);
			addEntry(grid, 2, 1, "", &m_new_product, true);
			addLabel(grid, 3, 0, "Manufacturer", nullptr, false);
			addEntry(grid, 3, 1, "", &m_new_manufacturer, true);

			hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
			gtk_box_pack_start(GTK_BOX(vbox), hbox, false, true, 0);
			gtk_widget_set_halign(hbox, GTK_ALIGN_END);

			apply = gtk_button_new_with_label("Apply");
			g_signal_connect(apply, "clicked", G_CALLBACK(onDescriptionApply), this);
			gtk_box_pack_start(GTK_BOX(hbox), apply, false, false, 0);
			GtkWidget *load_defaults = gtk_button_new_with_label("Load defaults");
			g_signal_connect(load_defaults, "clicked", G_CALLBACK(onLoadDefaults), this);
			gtk_box_pack_start(GTK_BOX(hbox), load_defaults, false, false, 0);
			reload = gtk_button_new_with_label("Reload");
			g_signal_connect(reload, "clicked", G_CALLBACK(onReload), this);
			gtk_box_pack_start(GTK_BOX(hbox), reload, false, false, 0);
			refresh = gtk_button_new_with_label("Refresh device list");
			g_signal_connect(refresh, "clicked", G_CALLBACK(onRefresh), this);
			gtk_box_pack_start(GTK_BOX(hbox), refresh, false, false, 0);

			grid = addPageWithGrid("Configuration");
			gtk_widget_set_vexpand(grid, true);
			vbox = gtk_widget_get_parent(grid);
			string vid = toHexString(m_configuration.vid);
			string pid = toHexString(m_configuration.pid);

			addLabel(grid, 0, 0, "Vendor ID", nullptr, false);
			addEntry(grid, 0, 1, vid.c_str(), &m_vid, true);
			addLabel(grid, 1, 0, "Product ID", nullptr, false);
			addEntry(grid, 1, 1, pid.c_str(), &m_pid, true);
			addLabel(grid, 2, 0, "Default product", nullptr, false);
			addEntry(grid, 2, 1, m_configuration.product.c_str(), &m_product, true);
			addLabel(grid, 3, 0, "Default manufacturer", nullptr, false);
			addEntry(grid, 3, 1, m_configuration.manufacturer.c_str(), &m_manufacturer, true);

			hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
			gtk_box_pack_start(GTK_BOX(vbox), hbox, false, true, 0);
			gtk_widget_set_halign(hbox, GTK_ALIGN_END);

			apply = gtk_button_new_with_label("Apply");
			g_signal_connect(apply, "clicked", G_CALLBACK(onConfigurationApply), this);
			gtk_box_pack_start(GTK_BOX(hbox), apply, false, false, 0);

			gtk_box_pack_start(GTK_BOX(vbox_main), m_notebook, true, true, 0);
			g_signal_connect(G_OBJECT(m_notebook), "switch-page", G_CALLBACK(onPageSwitch), this);

			m_usb_event_thread = thread(&Impl::waitForUsbEvents, this);
			showDevices();
			gtk_widget_show_all(vbox_main);
			gtk_container_add(GTK_CONTAINER(m_window), vbox_main);
			gtk_widget_show(m_window);
			gtk_main();
			return 0;
		}
		void pageSwitch(int page)
		{
			switch (page){
				case 0:
					if (gtk_widget_get_parent(m_device_list) != m_device_scrolled_widget){
						g_object_ref(m_device_list);
						gtk_container_remove(GTK_CONTAINER(m_description_scrolled_widget), m_device_list);
						gtk_container_add(GTK_CONTAINER(m_device_scrolled_widget), m_device_list);
						g_object_unref(m_device_list);
					}
					break;
				case 1:
					if (gtk_widget_get_parent(m_device_list) != m_description_scrolled_widget){
						g_object_ref(m_device_list);
						gtk_container_remove(GTK_CONTAINER(m_device_scrolled_widget), m_device_list);
						gtk_container_add(GTK_CONTAINER(m_description_scrolled_widget), m_device_list);
						g_object_unref(m_device_list);
					}
					break;
			}
		}
		void applyConfiguration()
		{
			m_configuration.vid = toUint16(gtk_entry_get_text(GTK_ENTRY(m_vid)));
			m_configuration.pid = toUint16(gtk_entry_get_text(GTK_ENTRY(m_pid)));
			m_configuration.manufacturer = gtk_entry_get_text(GTK_ENTRY(m_manufacturer));
			m_configuration.product = gtk_entry_get_text(GTK_ENTRY(m_product));
			showDevices();
		}
		void applyDescription()
		{
			uint16_t vid = toUint16(gtk_entry_get_text(GTK_ENTRY(m_new_vid)));
			uint16_t pid = toUint16(gtk_entry_get_text(GTK_ENTRY(m_new_pid)));
			string manufacturer = gtk_entry_get_text(GTK_ENTRY(m_new_manufacturer));
			string product = gtk_entry_get_text(GTK_ENTRY(m_new_product));
			mcp2200::Device device;
			if (!device.open(m_current_device.c_str())){
				return;
			}
			std::string old_manufacturer, old_product;
			device.getManufacturer(old_manufacturer);
			device.getProduct(old_product);
			if (old_manufacturer != manufacturer)
				device.setManufacturer(manufacturer.c_str());
			if (old_product != product)
				device.setProduct(product.c_str());
			if (m_configuration.vid != vid || m_configuration.pid != pid)
				device.setVendorProductIds(vid, pid);
			showDevices();
		}
		void loadDefaults()
		{
			string vid = toHexString(m_configuration.vid);
			string pid = toHexString(m_configuration.pid);
			gtk_entry_set_text(GTK_ENTRY(m_new_vid), vid.c_str());
			gtk_entry_set_text(GTK_ENTRY(m_new_pid), pid.c_str());
			gtk_entry_set_text(GTK_ENTRY(m_new_manufacturer), m_configuration.manufacturer.c_str());
			gtk_entry_set_text(GTK_ENTRY(m_new_product), m_configuration.product.c_str());
		}
		static gboolean onDeleteEvent(GtkWidget *, GdkEvent *, Program *)
		{
			return false;
		}
		void waitForUsbEvents()
		{
#ifdef LINUX_BUILD
			Udev udev;
			udev.open();
			bool exit = false;
			{
				lock_guard<mutex> lock(m_before_close_mutex);
				m_before_close = [&udev, &exit](){
					exit = true;
					udev.interruptRead();
				};
			}
			while (!exit){
				udev.read([this](const char *, const char *, const char *){
					g_idle_add((GSourceFunc)&Impl::onUsbEvent, this);
				});
			}
			{
				lock_guard<mutex> lock(m_before_close_mutex);
				m_before_close = []{};
			}
#endif
		}
		static gboolean onUsbEvent(Impl *app)
		{
			app->showDevices();
			return false;
		}
		static void onCursorChanged(GtkTreeView *, Impl *)
		{
		}
		static void onRowActivated(GtkTreeView *list, GtkTreePath *tree_path, GtkTreeViewColumn *, Impl *app)
		{
			GtkTreeModel* model = gtk_tree_view_get_model(list);
			GtkTreeIter iter;
			gtk_tree_model_get_iter(model, &iter, tree_path);
			gchar *path = nullptr;
			gtk_tree_model_get(model, &iter, enum_value(DeviceColumn::path), &path, -1);
			if (path){
				app->openDevice(path);
				g_free(path);
			}
		}
		static void onReload(GtkWidget *, Impl *app)
		{
			app->openDevice();
		}
		static void onApply(GtkWidget *, Impl *app)
		{
			app->applyToDevice();
		}
		static void onApplyGpioValues(GtkWidget *, Impl *app)
		{
			app->applyGpioValuesToDevice();
		}
		static void onRefresh(GtkWidget *, Impl *app)
		{
			app->showDevices();
		}
		static void onChange(GtkWidget *, Impl *app)
		{
			app->getGpioMask();
		}
		static void onConfigurationApply(GtkWidget *, Impl *app)
		{
			app->applyConfiguration();
		}
		static void onDescriptionApply(GtkWidget *, Impl *app)
		{
			app->applyDescription();
		}
		static void onPageSwitch(GtkWidget *, GtkWidget *, guint page_num, Impl *app)
		{
			app->pageSwitch(page_num);
		}
		static void onLoadDefaults(GtkWidget *, Impl *app)
		{
			app->loadDefaults();
		}
	};
	Program::Program()
	{
		hid_init();
		m_impl.reset(new Impl(this));
	}
	Program::~Program()
	{
		hid_exit();
	}
	int Program::run(int argc, char **argv)
	{
		return m_impl->run(argc, argv);
	}
}
