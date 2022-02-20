#pragma once
#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <array>
#include <stdexcept>
#include <utility>

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


// Марина М. если std::move(Type()) заменить на Type{}, то получится практически реализация 
//           конструктора со значением, можно делегировать работу ему. Все-таки Type{} - это тоже некое значение

    /*
    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : size_(size), capacity_(size), internal_array_(size)
    {
        //Напишите тело конструктора самостоятельно
        //std::fill(internal_array_.Get(), internal_array_.Get() + size, std::move(Type()));
        CustomFill(internal_array_.Get(), internal_array_.Get() + size);
    }
    */

    explicit SimpleVector(size_t size) : SimpleVector(size, Type{})
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size), internal_array_(size)
    {
        std::fill(internal_array_.Get(), internal_array_.Get() + size, value);
    }

    // Создаёт вектор из size элементов, инициализированных значением rvalue
    SimpleVector(size_t size, Type&& rvalue) : size_(size), capacity_(size), internal_array_(size)
    {
        CustomFill(internal_array_.Get(), internal_array_.Get() + size);
        *begin() = std::move(rvalue);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : size_(init.size()), capacity_(init.size()), internal_array_(init.size())
    {
        // Перемещаем элементы, т.к. они могут быть без конструктора копирования
        static_cast<void>(std::move(init.begin(), init.end(), internal_array_.Get()));
    }

    SimpleVector(const SimpleVector& other)
    {
        // Допущение: временный вектор создается с capacity==size, а не с capacity==other.capacity
        SimpleVector tmp_vector(other.size_);
        std::copy(other.begin(), other.end(), tmp_vector.begin());
        SimpleVector::swap(tmp_vector);
    }

    SimpleVector& operator=(const SimpleVector& rhs)
    {
        if (this != &rhs)
        {
            auto tmp_rhs(rhs);
            SimpleVector::swap(tmp_rhs);
        }
        return *this;
    }

    // Конструктор перемещения
    SimpleVector(SimpleVector&& other) noexcept : size_(other.size_), capacity_(other.capacity_)
    {
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
    SimpleVector& operator=(SimpleVector&& rhs) noexcept
    {
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
        // Т.к. этот конструктор создает вектор с заданной емкостью, но с размером 0, 
        // вызывать fill не требуется (нет элементов для заполнения)
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept
    {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept
    {
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept
    {

// Марина М. в реализации не хватает проверки, что индекс не превышает размерности вектора 
        assert(index >= 0 && index < size_);

// Марина М. в ArrayPtr у вас реализованы квадратные скобки, поэтому можно их использовать 
        //return *(internal_array_.Get() + index);
        return internal_array_.Get()[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept
    {

// Марина М. в реализации не хватает проверки, что индекс не превышает размерности вектора 
        assert(index >= 0 && index < size_);

        // Марина М. в ArrayPtr у вас реализованы квадратные скобки, поэтому можно их использовать 
        //return *(internal_array_.Get() + index);
        return internal_array_.Get()[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index)
    {
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
        if (index >= size_)
        {
            throw out_of_range("Index is out of range (operator const At())"s);
        }
        return *(internal_array_.Get() + index);
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept
    {
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
                CustomFill(internal_array_.Get() + size_, internal_array_.Get() + new_size);
            }
            else
            {
                // Новый размер больше вместимости. Реалоцируем память под более емкий массив
                const size_t new_capacity = std::max(new_size, 2 * capacity_);
                ArrayPtr<Type> buffer(new_capacity);

                // Обнуляем новые значащие элементы [size...new_size)
                CustomFill(buffer.Get() + size_, buffer.Get() + new_size);
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

        if (size_ < capacity_)
        {
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

        if (size_ < capacity_)
        {
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

// Марина М. в метода Insert и Erase не хватает проверки (assert), что указанная позиция 
// попадает в интервал от начала до конца вектора
        assert(pos >= begin() && pos <= end());

        auto offset_start = std::distance(cbegin(), pos);

        if (size_ < capacity_)
        {
            // Сдвигаем элементы по одному к концу вектора начиная с конца, освобождая pos
            std::copy_backward(pos, cend(), end() + 1);
            // Вставляем value
            internal_array_[offset_start] = value;
            ++size_;
        }
        else
        {
            // Выделяем новый массив с удвоенной емкостью
            const size_t new_capacity = (size_ > 0 ? 2 * capacity_ : 1);
            ArrayPtr<Type> buffer(new_capacity);

            // Копируем начало вектора до точки вставки
            std::copy(begin(), Iterator(pos), buffer.Get());
            // Вставляем добавляемое значение в позицию pos
            buffer[offset_start] = value;
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

// Марина М. в метода Insert и Erase не хватает проверки (assert), что указанная позиция 
// попадает в интервал от начала до конца вектора
        assert(pos >= begin() && pos <= end());

        auto offset_start = std::distance(cbegin(), pos);

        if (size_ < capacity_)
        {
            // Сдвигаем элементы по одному к концу вектора начиная с конца, освобождая pos
            std::move_backward(Iterator(pos), Iterator(cend()), Iterator(end() + 1));
            // Используем семантику перемещения для некопируемых value
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

// Марина М. стоит проверить по ассерт. так как серьезная ошибка - попытка удалить запись из пустого вектора
/*        if (!IsEmpty())
        {
            --size_;
        }*/
        assert(!IsEmpty());
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos)
    {

// Марина М. в метода Insert и Erase не хватает проверки (assert), что указанная позиция 
// попадает в интервал от начала до конца вектора
        assert(pos >= begin() && pos <= end());

        // Сдвигаем элементы после pos по одному к началу вектора, затирая pos
        // Используем move итераторы
        std::move(Iterator(pos + 1), Iterator(cend()), Iterator(pos));
        --size_;
        return Iterator(pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept
    {
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
        if (new_capacity > capacity_)
        {
            ArrayPtr<Type> buffer(new_capacity);
            std::move(internal_array_.Get(), internal_array_.Get() + size_, buffer.Get());
            // Оптимизация: не используем fill для области резервной памяти (size_...capacity_)
            internal_array_.swap(buffer);
            capacity_ = new_capacity;
            // size_ не изменяем, т.к. количество элементов не меняется
        }
    }


    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept
    {
        return internal_array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept
    {
        return internal_array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept
    {
        return internal_array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept
    {
        return internal_array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept
    {
        return internal_array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept
    {
        return internal_array_.Get() + size_;
    }

private:
    size_t size_ = 0;         // Количество элементов в векторе
    size_t capacity_ = 0;     // Выделено памяти в векторе (элементов)

    // Внутренний массив, управляемый умным указателем
    ArrayPtr<Type> internal_array_;

// Марина М. стиль наименования функции не совпадает с единым стилем в реализации
// ----Станислав М. ОК. Стиль подражал стилю std::fill, std:copy_backward и т.д.
    // Заполнение диапазона некопируемыми типами
    void CustomFill(Iterator first, Iterator last)
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
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    // выражение SimpleVector1 <= SimpleVector2 противоположно SimpleVector2 < SimpleVector1
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    // выражение SimpleVector1 > SimpleVector2 эквивалентно SimpleVector2 < SimpleVector1
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    // выражение SimpleVector1 >= SimpleVector2 противоположно SimpleVector2 > SimpleVector1
    return !(rhs > lhs);
}
