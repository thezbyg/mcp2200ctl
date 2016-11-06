/*
Copyright (c) 2016, Albertas Vyšniauskas
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
#ifndef HEADER_MAPPING_H_
#define HEADER_MAPPING_H_
#include <json/json.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
namespace mapping
{
	struct FieldNotFound: public std::runtime_error
	{
		FieldNotFound():
			std::runtime_error("Field not found")
		{
		}
	};
	struct WrongType: public std::runtime_error
	{
		WrongType():
			std::runtime_error("Wrong type")
		{
		}
	};
	enum class Status
	{
		none,
		field_not_found,
		wrong_type,
	};
	struct Layout;
	struct BaseValue
	{
		std::string name;
		size_t offset, item_size, count;
		const char *type;
		BaseValue(const char *name, size_t offset, const char *type):
			name(name),
			offset(offset),
			item_size(0),
			count(0),
			type(type)
		{
		}
		BaseValue(const char *name, size_t offset, size_t item_size, size_t count, const char *type):
			name(name),
			offset(offset),
			item_size(item_size),
			count(count),
			type(type)
		{
		}
		virtual ~BaseValue() = default;
		const std::string getName() const
		{
			return name;
		}
		const char *getType() const
		{
			return type;
		}
		const size_t getOffset() const
		{
			return offset;
		}
		const size_t getCount() const
		{
			return count;
		}
		const size_t getItemSize() const
		{
			return item_size;
		}
		const bool isArray() const
		{
			return count > 1 && item_size != 0;
		}
		void *getValuePointer(void *object) const
		{
			return (reinterpret_cast<uint8_t*>(object) + offset);
		}
		const void *getValuePointer(const void *object) const
		{
			return (reinterpret_cast<const uint8_t*>(object) + offset);
		}
		void *getValuePointer(void *object, size_t index) const
		{
			return (reinterpret_cast<uint8_t*>(object) + offset + item_size * index);
		}
		const void *getValuePointer(const void *object, size_t index) const
		{
			return (reinterpret_cast<const uint8_t*>(object) + offset + item_size * index);
		}
		template <typename T>
		T *getValuePointer(void *object) const
		{
			return reinterpret_cast<T*>(getValuePointer(object));
		}
		template <typename T>
		const T *getValuePointer(const void *object) const
		{
			return reinterpret_cast<const T*>(getValuePointer(object));
		}
		template <typename T>
		const T &get(const void *object) const
		{
			return *getValuePointer<T>(object);
		}
		template <typename T>
		T &get(void *object) const
		{
			return *getValuePointer<T>(object);
		}
		template <typename T>
		void set(void *object, T &value) const
		{
			*getValuePointer<T>(object) = value;
		}
		template <typename T>
		void set(void *object, T value) const
		{
			*getValuePointer<T>(object) = value;
		}
		virtual void setDefault(void *object) const = 0;
	};
	template <typename T>
	struct Value: public BaseValue
	{
		Value(const char *name, size_t offset, const char *type):
			BaseValue(name, offset, type)
		{
		}
		virtual ~Value() {}
		T *getValuePointer(void *object) const
		{
			return reinterpret_cast<T*>(BaseValue::getValuePointer(object));
		}
		const T *getValuePointer(const void *object) const
		{
			return reinterpret_cast<const T*>(BaseValue::getValuePointer(object));
		}
		const T &get(const void *object) const
		{
			return *getValuePointer(object);
		}
		T &get(void *object)
		{
			return *getValuePointer(object);
		}
		void set(void *object, T &value)
		{
			*getValuePointer(object) = value;
		}
		void set(void *object, T value)
		{
			*getValuePointer(object) = value;
		}
	};
	template <typename T>
	struct Adapter
	{
		const char *type;
		Adapter(const char *type):
			type(type)
		{
		}
		virtual ~Adapter() = default;
		const char *getType() const
		{
			return type;
		}
		virtual void save(const void *object, const BaseValue &value, T &output) = 0;
		virtual void load(void *object, const BaseValue &value, const T &input) = 0;
	};
	struct Layout
	{
		const char *type;
		std::vector<std::unique_ptr<BaseValue>> values;
		std::map<std::string, size_t> value_map;
		std::map<std::string, std::shared_ptr<Layout>> inner_layout_map;
		Layout():
			type(nullptr)
		{
		}
		Layout(const char *type):
			type(type)
		{
		}
		const char *getType() const
		{
			return type;
		}
		Layout &operator()(std::unique_ptr<BaseValue> &&value)
		{
			value_map[value->getName()] = values.size();
			values.push_back(std::move(value));
			return *this;
		}
		Layout &operator()(BaseValue *value)
		{
			value_map[value->getName()] = values.size();
			values.push_back(std::unique_ptr<BaseValue>(value));
			return *this;
		}
		Layout &operator()(std::shared_ptr<Layout> layout)
		{
			inner_layout_map[layout->getType()] = layout;
			return *this;
		}
	};
	template <typename T>
	struct ReaderWriter
	{
		std::map<const char*, std::unique_ptr<Adapter<T>>> adapter_map;
		ReaderWriter &operator()(std::unique_ptr<Adapter<T>> &&adapter)
		{
			adapter_map[adapter->getType()] = std::move(adapter);
			return *this;
		}
		ReaderWriter &operator()(Adapter<T> *adapter)
		{
			adapter_map[adapter->getType()] = std::move(std::unique_ptr<Adapter<T>>(adapter));
			return *this;
		}
		void save(const Layout &layout, const void *object, T &output)
		{
			for (auto &value: layout.values){
				auto adapter = adapter_map.find(value->getType());
				if (adapter != adapter_map.end()){
					adapter->second->save(object, *value, output);
				}else{
					auto inner_layout = layout.inner_layout_map.find(value->getType());
					if (inner_layout != layout.inner_layout_map.end()){
						save(*inner_layout->second, value->getValuePointer(object), output[value->getName()]);
					}else{
						std::cerr << "Unhandled field " << value->getName() << " of type " << value->getType() << std::endl;
					}
				}
			}
		}
		void load(const Layout &layout, const T &input, void *object)
		{
			for (auto &value: layout.values){
				auto adapter = adapter_map.find(value->getType());
				if (adapter != adapter_map.end()){
					adapter->second->load(object, *value, input);
				}else{
					auto inner_layout = layout.inner_layout_map.find(value->getType());
					if (inner_layout != layout.inner_layout_map.end()){
						load(*inner_layout->second, input[value->getName()], value->getValuePointer(object));
					}
				}
			}
		}
	};
}
#endif /* HEADER_MAPPING_H_ */
