#pragma once

#include "array_ptr.h"

#include <algorithm>
#include <iostream>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>

class ReserveProxyObj {
public: 
    ReserveProxyObj(size_t capacity) 
    : reserved_capacity(capacity) {}
    
    size_t GetCapacity() const {
        return reserved_capacity;
    }
private:
    size_t reserved_capacity;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};
    
template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;
    
    SimpleVector() noexcept = default;
    
    SimpleVector(const ReserveProxyObj& obj) 
        : size_(0u), capacity_(obj.GetCapacity()), data_(obj.GetCapacity())
    {
    }
    
    // перемещение
    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) 
        : size_(size), capacity_(size), data_(size) {
            std::generate(data_.Get(), data_.Get() + size, [](){return std::move(Type());});
        }
    
    // копирование
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
        : size_(size), capacity_(size), data_(size) {
            std::fill(data_.Get(), data_.Get() + size, value);
        }    
    
    // перемещение
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, Type&& value) 
        : size_(size), capacity_(size), data_(size) {
            std::generate(data_.Get(), data_.Get() + size, [&value]() {return std::move(value);});
        }

    // копирование
    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()), capacity_(init.size()), data_(init.size()) {
            std::copy(init.begin(), init.end(), data_.Get());
        }
        
    // копирование
    SimpleVector(const SimpleVector& other) {
        SimpleVector<Type> tmp_vector(other.GetSize());
        std::copy(other.begin(), other.end(), tmp_vector.begin());
        swap(tmp_vector);
    }
    
    // перемещение
    SimpleVector(SimpleVector&& other) {
        swap(other);
    }
    
    // копирование
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    } 

    // перемещение
    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
    if (this != &rhs) {
        data_ = std::move(rhs.data_);
        size_ = std::move(rhs.size_);
        capacity_ = std::move(rhs.capacity_);
    }
    return *this;
}
    
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index out of range");
        }
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index out of range");
        }
        return data_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }
    
    // по значению 
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_data(new_capacity);
            std::move(data_.Get(), data_.Get() + size_, new_data.Get());
            data_.swap(new_data);
            capacity_ = new_capacity;
        }
    }
    
    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else if (new_size <= capacity_) {
            std::generate(data_.Get() + size_, data_.Get() + new_size, [](){return Type();});
            size_ = new_size;
        } else {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> new_data(new_capacity);
            std::move(data_.Get(), data_.Get() + size_, new_data.Get());
            std::generate(new_data.Get() + size_, new_data.Get() + new_size, [](){return Type();});
            data_.swap(new_data);
            capacity_ = new_capacity;
            size_ = new_size;
        }
    }
    
    // копирование 
    // Добавляет элемент в конец вектора 
    // При нехватке места увеличивает вдвое вместимость вектора 
    void PushBack(const Type& item) { 
        if (size_ == capacity_) { 
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2); 
            ArrayPtr<Type> new_data(new_capacity); 
            std::move(data_.Get(), data_.Get() + size_, new_data.Get());
            *(new_data.Get() + size_) = std::move(item); 
            data_.swap(new_data); 
            capacity_ = new_capacity; 
            ++size_; 
        } else { 
            *(data_.Get() + size_) = std::move(item);
            ++size_; 
        }
    }
    
    // перемещение
    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            ArrayPtr<Type> new_data(new_capacity);
            std::move(data_.Get(), data_.Get() + size_, new_data.Get());
            *(new_data.Get() + size_) = std::move(item);
            data_.swap(new_data);
            capacity_ = new_capacity;
            ++size_;
        } else {
            *(data_.Get() + size_) = std::move(item);
            ++size_;
        }
    }
    
    // копирование
    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        auto it = pos - data_.Get();
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            ArrayPtr<Type> new_data(new_capacity);
            std::copy(data_.Get(), data_.Get() + it, new_data.Get());
            *(new_data.Get() + it) = value;
            std::copy(data_.Get() + it, data_.Get() + size_, new_data.Get() + it + 1);
            data_.swap(new_data);
            capacity_ = new_capacity;
            ++size_;
        } else {
            std::copy_backward(data_.Get() + it, data_.Get() + size_, data_.Get() + size_ + 1);
            *(data_.Get() + it) = value;
            ++size_;
        }
        return data_.Get() + it;
    }
    
    // перемещение
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        auto it = pos - data_.Get();
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0 ? 1 : capacity_ * 2);
            ArrayPtr<Type> new_data(new_capacity);
            std::move(data_.Get(), data_.Get() + it, new_data.Get());
            *(new_data.Get() + it) = std::move(value);
            std::move(data_.Get() + it, data_.Get() + size_, new_data.Get() + it + 1);
            data_.swap(new_data);
            capacity_ = new_capacity;
            ++size_;
        } else {
            std::move_backward(data_.Get() + it, data_.Get() + size_, data_.Get() + size_ + 1);
            *(data_.Get() + it) = std::move(value);
            ++size_;
        }
        return data_.Get() + it;
    }
    
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;    
    }
    
    // копирование
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {        
        assert(pos >= begin() && pos < end());
        auto it = pos - data_.Get();
        std::copy(std::make_move_iterator(data_.Get() + it + 1), std::make_move_iterator(data_.Get() + size_), data_.Get() + it);
        --size_;
        return data_.Get() + it;
    }
    
    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        data_.swap(other.data_);
    }
    
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return data_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return data_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return data_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return end();
    }
private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> data_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
