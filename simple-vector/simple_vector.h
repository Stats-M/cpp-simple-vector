// вставьте сюда ваш код для класса SimpleVector
// внесите необходимые изменения для поддержки move-семантики
#pragma once
#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <array>
#include <stdexcept>
#include <utility>

// Для отладочных выводов, в финале можно отключить
//#include <iostream>

using namespace std;

// Класс-обертка для различия версий конструкторов с параметрами size и reserve (оба типа size_t)
class ReserveProxyObj
{
public:
    ReserveProxyObj() = delete;

    explicit ReserveProxyObj(size_t capacity) : reserve_value(capacity)
    {}

    size_t reserve_value = 0;
};

// Функция, создающая прокси-объект для передачи его в конструктор SimpleVector
ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector
{
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : size_(size), capacity_(size), internal_array_(size)
    {
        //Напишите тело конструктора самостоятельно
        //std::fill(internal_array_.Get(), internal_array_.Get() + size, std::move(Type()));
        custom_fill(internal_array_.Get(), internal_array_.Get() + size);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size), internal_array_(size)
    {
        //Напишите тело конструктора самостоятельно
        std::fill(internal_array_.Get(), internal_array_.Get() + size, value);
    }

    // Создаёт вектор из size элементов, инициализированных значением rvalue
    SimpleVector(size_t size, Type&& rvalue) : size_(size), capacity_(size), internal_array_(size)
    {
        // Напишите тело конструктора с инициализацией rvalue самостоятельно
        //std::fill(internal_array_.Get(), internal_array_.Get() + size, std::move(Type()));
        custom_fill(internal_array_.Get(), internal_array_.Get() + size);
        *begin() = std::move(rvalue);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : size_(init.size()), capacity_(init.size()), internal_array_(init.size())
    {
        //Напишите тело конструктора самостоятельно

        // Перемещаем элементы, т.к. они могут быть без конструктора копирования
        static_cast<void>(std::move(init.begin(), init.end(), internal_array_.Get()));
    }

    SimpleVector(const SimpleVector& other)
    {
        // Напишите тело конструктора самостоятельно

        // Допущение: временный вектор создается с capacity==size, а не с capacity==other.capacity
        SimpleVector tmp_vector(other.size_);
        std::copy(other.begin(), other.end(), tmp_vector.begin());
        SimpleVector::swap(tmp_vector);
    }

    SimpleVector& operator=(const SimpleVector& rhs)
    {
        // Напишите тело конструктора самостоятельно

        if (this != &rhs)
        {
            auto tmp_rhs(rhs);
            SimpleVector::swap(tmp_rhs);
        }
        return *this;
    }

    // Конструктор перемещения
    SimpleVector(SimpleVector&& other) : size_(other.size_), capacity_(other.capacity_)
    {
        //Напишите тело конструктора перемещения самостоятельно

        if (this != &other)
        {
            internal_array_.swap(other.internal_array_);
            std::swap(this->size_, other.size_);
            std::swap(this->capacity_, other.capacity_);
            // Очищаем аргумент (делаем перемещение)
            other.Clear();
        }

    }

    // Конструктор присваивания перемещением
    SimpleVector& operator=(SimpleVector&& rhs)
    {
        // Напишите тело конструктора присваивания перемещением самостоятельно
        if (this != &rhs)
        {
            internal_array_.swap(rhs.internal_array_);
            std::swap(this->size_, rhs.size_);
            std::swap(this->capacity_, rhs.capacity_);
            // Очищаем аргумент (делаем перемещение)
            rhs.Clear();
        }

        // Возвращаем *this во всех случаях
        return *this;
    }

    SimpleVector(ReserveProxyObj obj) : size_(0u), capacity_(obj.reserve_value), internal_array_(capacity_)
    {
        //Напишите тело конструктора самостоятельно
        //std::fill(internal_array_.Get(), internal_array_.Get() + capacity_, Type{});

        // Т.к. этот конструктор создает вектор с заданной емкостью, но с размером 0, 
        // вызывать fill не требуется (нет элементов для заполнения)
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept
    {
        //Напишите тело самостоятельно
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept
    {
        //Напишите тело самостоятельно
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept
    {
        //Напишите тело самостоятельно
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept
    {
        //Напишите тело самостоятельно
        return *(internal_array_.Get() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept
    {
        //Напишите тело самостоятельно
        return *(internal_array_.Get() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index)
    {
        //Напишите тело самостоятельно
        if (index >= size_)
        {
            throw out_of_range("Index is out of range (operator At())"s);
        }
        return *(internal_array_.Get() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const
    {
        //Напишите тело самостоятельно
        if (index >= size_)
        {
            throw out_of_range("Index is out of range (operator const At())"s);
        }
        return *(internal_array_.Get() + index);
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept
    {
        //Напишите тело самостоятельно
        size_ = 0;
    }

    // Изменяет размер массива (количество элементов со значениями). size <= capacity
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
        // Изменяем размер только когда новый размер больше старого
        if (new_size > size_)
        {
            // Новый размер меньше емкости. Просто обнуляем добавляемые сверх size_ элементы
            if (new_size <= capacity_)
            {
                // Не вычитаем 1 из [internal_array_.Get() + new_size), т.к. правая граница 
                // полуинтервала не входит в интервал обрабатываемых алгоритмом данных
                //std::fill(internal_array_.Get() + size_, internal_array_.Get() + new_size, Type());
                custom_fill(internal_array_.Get() + size_, internal_array_.Get() + new_size);
            }
            else
            {
                // Новый размер больше вместимости. Реалоцируем память под более емкий массив
                const size_t new_capacity = std::max(new_size, 2 * capacity_);
                ArrayPtr<Type> buffer(new_capacity);

                // Обнуляем новые значащие элементы [size...new_size)
                //std::fill(buffer.Get() + size_, buffer.Get() + new_size, Type());
                custom_fill(buffer.Get() + size_, buffer.Get() + new_size);
                // Перемещаем старые элементы (move т.к. могут быть элементы без конструктора копирования)
                std::move(internal_array_.Get(), internal_array_.Get() + size_, buffer.Get());
                // Меняем указатели на массивы 
                internal_array_.swap(buffer);
                capacity_ = new_capacity;
            }
        }

        // Запоминаем новый размер вектора.
        size_ = new_size;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item)
    {
        // Напишите тело PushBack самостоятельно

        if (size_ < capacity_)
        {
            //internal_array_[size_] = item;
            std::exchange(internal_array_[size_], item);
            ++size_;
        }
        else
        {
            // Выделяем новый массив с удвоенной емкостью
            const size_t new_capacity = (capacity_ > 0 ? 2 * capacity_ : 1);
            ArrayPtr<Type> buffer(new_capacity);

            // Копируем исходный вектор во временный буфер
            std::copy(internal_array_.Get(), internal_array_.Get() + size_, buffer.Get());
            // Копируем добавляемое значение в конец буфера
            //buffer[size_] = item;
            std::exchange(buffer[size_], item);
            // Меняем указатели на массивы 
            buffer.swap(internal_array_);
            capacity_ = new_capacity;
            ++size_;
        }

    }

    // Добавляет элемент (rvalue) в конец вектора перемещением
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(Type&& item)
    {
        // Напишите тело PushBack самостоятельно

        if (size_ < capacity_)
        {
            //internal_array_[size_] = std::move(item);
            std::exchange(internal_array_[size_], std::move(item));
            ++size_;
        }
        else
        {
            // Выделяем новый массив с удвоенной емкостью
            const size_t new_capacity = (capacity_ > 0 ? 2 * capacity_ : 1);
            ArrayPtr<Type> buffer(new_capacity);

            // Перемещаем исходный вектор во временный буфер
            std::move(internal_array_.Get(), internal_array_.Get() + size_, buffer.Get());
            // Перемещаем добавляемое значение в конец буфера
            //buffer[size_] = std::move(item);
            std::exchange(buffer[size_], std::move(item));
            // Меняем указатели на массивы 
            buffer.swap(internal_array_);
            capacity_ = new_capacity;
            ++size_;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value)
    {
        // Напишите тело Insert самостоятельно
        auto offset_start = std::distance(cbegin(), pos);

        if (size_ < capacity_)
        {
            // Сдвигаем элементы по одному к концу вектора начиная с конца, освобождая pos
//            std::move_backward(pos, cend(), end() + 1);
            std::copy_backward(pos, cend(), end() + 1);
            // Вставляем value
            internal_array_[offset_start] = value;
//            std::exchange(internal_array_[offset_start], value);
            ++size_;
        }
        else
        {
            // Выделяем новый массив с удвоенной емкостью
            const size_t new_capacity = (size_ > 0 ? 2 * capacity_ : 1);
            ArrayPtr<Type> buffer(new_capacity);

            // Копируем начало вектора до точки вставки
//            std::move(begin(), Iterator(pos), buffer.Get());
            std::copy(begin(), Iterator(pos), buffer.Get());
            // Вставляем добавляемое значение в позицию pos
            buffer[offset_start] = value;
//            std::exchange(buffer[offset_start], value);
            // Копируем оставшуюся часть вектора
            // buffer.Get() + отступ до места вставки + 1 вставленное значение
            std::copy(pos, cend(), buffer.Get() + offset_start + 1);
            // Меняем указатели на массивы 
            internal_array_.swap(buffer);
            capacity_ = new_capacity;
            ++size_;
        }

        return Iterator{ &internal_array_[offset_start] };
    }

    // Вставляет значение value (rvalue) в позицию pos перемещением.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, Type&& value)
    {
        // Напишите тело Insert самостоятельно
        auto offset_start = std::distance(cbegin(), pos);
        // auto offset_end = std::distance(pos, cend());

        if (size_ < capacity_)
        {
            // Сдвигаем элементы по одному к концу вектора начиная с конца, освобождая pos
            std::move_backward(Iterator(pos), Iterator(cend()), Iterator(end() + 1));
            // Используем семантику перемещения для некопируемых value
            //internal_array_[offset_start] = std::move(value);
            std::exchange(internal_array_[offset_start], std::move(value));
            ++size_;
        }
        else
        {
            // Выделяем новый массив с удвоенной емкостью
            const size_t new_capacity = (size_ > 0 ? 2 * capacity_ : 1);
            ArrayPtr<Type> buffer(new_capacity);

            // Перемещаем начало вектора до точки вставки во временный буфер
            std::move(begin(), Iterator(pos), buffer.Get());
            // Перемещаем добавляемое значение в позицию pos
            //buffer[offset_start] = std::move(value);
            std::exchange(buffer[offset_start], std::move(value));
            // Перемещаем оставшуюся часть вектора
            // buffer.Get() + отступ до места вставки + 1 вставленное значение
            std::move(Iterator(pos), Iterator(cend()), Iterator(buffer.Get() + offset_start + 1));
            // Меняем указатели на массивы 
            internal_array_.swap(buffer);
            capacity_ = new_capacity;
            ++size_;
        }

        return Iterator{ &internal_array_[offset_start] };
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept
    {
        // Напишите тело PopBack самостоятельно
        if (!IsEmpty())
        {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos)
    {
        // Напишите тело Erase самостоятельно
        // Сдвигаем элементы после pos по одному к началу вектора, затирая pos
        //std::copy(pos + 1, cend(), Iterator(pos));
        // Используем move итераторы
        //std::copy(make_move_iterator(pos + 1), make_move_iterator(cend()), Iterator(pos));
        std::move(Iterator(pos + 1), Iterator(cend()), Iterator(pos));
        --size_;
        return Iterator(pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept
    {
        // Напишите тело swap самостоятельно

        // Проверка самоприсваивания (через адрес)
        if (this == &other)
        {
            return;
        }

        this->internal_array_.swap(other.internal_array_);    // меняет только массив
        std::swap(this->size_, other.size_);
        std::swap(this->capacity_, other.capacity_);
    }

    // Увеличивает емкость вектора
    void Reserve(size_t new_capacity)
    {
        // Напишите тело метода Reserve самостоятельно
        if (new_capacity > capacity_)
        {
            ArrayPtr<Type> buffer(new_capacity);
            //std::copy(internal_array_.Get(), internal_array_.Get() + size_, buffer.Get());
            std::move(internal_array_.Get(), internal_array_.Get() + size_, buffer.Get());
            // Оптимизация: не используем fill для области резервной памяти (size_...capacity_)
            //std::fill(buffer.Get() + size_, buffer.Get() + new_capacity, Type());
            internal_array_.swap(buffer);
            capacity_ = new_capacity;
            // size_ не изменяем, т.к. количество элементов не меняется
        }
    }


    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept
    {
        //Напишите тело самостоятельно
        return internal_array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept
    {
        //Напишите тело самостоятельно
        return internal_array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept
    {
        //Напишите тело самостоятельно
        return internal_array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept
    {
        //Напишите тело самостоятельно
        return internal_array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept
    {
        //Напишите тело самостоятельно
        return internal_array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept
    {
        //Напишите тело самостоятельно
        return internal_array_.Get() + size_;
    }

private:
    size_t size_ = 0;         // Количество элементов в векторе
    size_t capacity_ = 0;     // Выделено памяти в векторе (элементов)

    // Внутренний массив, управляемый умным указателем
    ArrayPtr<Type> internal_array_;

    // Заполнение диапазона некопируемыми типами
    void custom_fill(Iterator first, Iterator last)
    {
        assert(first < last);
        while (first < last)
        {
            *first = std::move(Type());
            ++first;
        }
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    // Заглушка. Напишите тело самостоятельно
    // return true;
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    // Заглушка. Напишите тело самостоятельно
    // return true;
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    // Заглушка. Напишите тело самостоятельно
    // return true;
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    // Заглушка. Напишите тело самостоятельно
    // return true;
    // выражение SimpleVector1 <= SimpleVector2 противоположно SimpleVector2 < SimpleVector1
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    // Заглушка. Напишите тело самостоятельно
    // return true;
    // выражение SimpleVector1 > SimpleVector2 эквивалентно SimpleVector2 < SimpleVector1
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    // Заглушка. Напишите тело самостоятельно
    // return true;
    // выражение SimpleVector1 >= SimpleVector2 противоположно SimpleVector2 > SimpleVector1
    return !(rhs > lhs);
}
