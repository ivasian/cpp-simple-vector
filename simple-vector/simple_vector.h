#pragma once

#include <cassert>
#include <initializer_list>
#include "array_ptr.h"
#include <algorithm>

struct ReserveProxyObj {
    ReserveProxyObj(size_t capacity_to_reserve) : capacity_to_reserve_(capacity_to_reserve){
    }
    size_t capacity_to_reserve_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}



template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;


    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        if(size > 0) {
            ArrayPtr<Type> temp (size);
            std::fill(temp.Get(), temp.Get()+size, Type());
            data_.swap(temp);
            size_ = size;
            capacity_ = size;
        }
    }
    SimpleVector(ReserveProxyObj obj) {
        ArrayPtr<Type> temp (obj.capacity_to_reserve_);
        capacity_ = obj.capacity_to_reserve_;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {
        if(size > 0) {
            ArrayPtr<Type> temp (size);
            std::fill(temp.Get(), temp.Get()+size, value);
            data_.swap(temp);
            size_ = size;
            capacity_ = size;
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        if(init.size() > 0){
            ArrayPtr<Type> temp (init.size());
            data_.swap(temp);
            capacity_ = init.size();
            Type* ptr = data_.Get();
            for(const Type& value : init ){
                *ptr++ = value;
            }
            size_ = init.size();
        }
    }

    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> temp (other.size_);
        auto ptr = temp.Get();
        for(const auto& value : other){
            *ptr++ = value;
        }
        data_.swap(temp);
        size_ = other.size_;
        capacity_ = other.capacity_;
    }

    SimpleVector(SimpleVector&& other) {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает количество элементов в массиве
    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    [[nodiscard]] size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return const_cast<const Type&> (data_[index]);
    }

    // Возвращает ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        using namespace std;
        if(index >= size_){
            throw (out_of_range("Index >= Array size"s));
        }
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    [[maybe_unused]] const Type& At(size_t index) const {
        using namespace std;
        if(index >= size_){
            throw (out_of_range("Index >= Array size"s));
        }
        return const_cast<const Type&> (data_[index]);;
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }


    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if(new_size < size_){
            size_ = new_size;
        } else if(new_size > size_){
            if(new_size > capacity_){
                Reserve(new_size);
            }
            auto It = data_.Get() + size_;
            while(It != data_.Get() + new_size){
                *It++ = std::move(Type());
            }
//            std::fill(data_.Get() + size_, data_.Get() + new_size, Type());
            size_ = new_size;
        }
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
    [[maybe_unused]] ConstIterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    [[maybe_unused]] ConstIterator cend() const noexcept {
        return end();
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if(this == &rhs) { return *this; }
        ArrayPtr<Type> temp (rhs.size_);
        Type* ptr = temp.Get();
        for(const auto& value : rhs){
            *ptr++ = value;
        }
        data_.swap(temp);
        capacity_ = rhs.size_;
        size_ = rhs.size_;
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if(capacity_ <= size_){
            Reserve(capacity_ == 0 ? 1 : capacity_*2);
        }
        data_[size_++] = item;
    }

    void PushBack(Type&& item) {
        if(capacity_ <= size_){
            Reserve(capacity_ == 0 ? 1 : capacity_*2);
        }
        data_[size_++] = std::move(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t index = pos - begin();
        if(capacity_ <= size_){
            Reserve(capacity_ == 0 ? 1 : capacity_*2);
        }
        auto It = const_cast <Iterator> (begin() + index);
        for(Iterator curr = end(); curr != It; --curr){
            *curr = *(curr - 1);
        }
        *It = value;
        ++size_;
        return It;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t index = pos - begin();
        if(capacity_ <= size_){
            Reserve(capacity_ == 0 ? 1 : capacity_*2);
        }
        auto It = const_cast <Iterator> (begin() + index);
        std::copy_backward(std::make_move_iterator(It), std::make_move_iterator(end()), end() + 1);
        *It = std::move(value);
        ++size_;
        return It;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if(size_ > 0){
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        Iterator It = const_cast <Iterator> (pos);
        std::copy(std::make_move_iterator(It + 1), std::make_move_iterator(end()), It);
        --size_;
        return const_cast <Iterator> (pos);;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity){
        if(new_capacity > capacity_){
            ArrayPtr<Type> temp(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), temp.Get());
            data_.swap(temp);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> data_;
    size_t size_ = 0, capacity_ = 0;
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
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
    return lhs < rhs || lhs == rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}