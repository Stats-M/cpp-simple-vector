// вставьте сюда ваш код для класса ArrayPtr
// внесиnте в него изменения, 
// которые позволят реализовать move-семантику

#pragma once

#include <cassert>
#include <cstdlib>
#include <string_view>    //для std::swap()
#include <utility>        //для std::exchange()

template <typename Type>
class ArrayPtr
{
public:
    // Инициализирует ArrayPtr нулевым указателем
    ArrayPtr() = default;

    // Создаёт в куче массив из size элементов типа Type.
    // Если size == 0, поле raw_ptr_ должно быть равно nullptr
    explicit ArrayPtr(size_t size)
    {
        if (size != 0)
        {
            raw_ptr_ = new Type[size];
        }
    }

    // Конструктор из сырого указателя, хранящего адрес массива в куче либо nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept
    {
        if (raw_ptr_ != nullptr)
        {
            delete[] raw_ptr_;
        }
        raw_ptr_ = raw_ptr;
    }

    // Запрещаем конструктор копирования
    ArrayPtr(const ArrayPtr&) = delete;

    // Запрещаем присваивание копированием
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    // Конструктор перемещения.
    // Забирает массив у объекта-источника.
    ArrayPtr(ArrayPtr&& source)
    {
        // Присваиваем указатель на массив
// Марина М.: std::exchange или свапнуть
//raw_ptr_ = source.raw_ptr_;
// Стираем указатель у источника
//source.raw_ptr_ = nullptr;
        raw_ptr_ = std::exchange(source.raw_ptr_, nullptr);
    }

    // Оператор присваивания перемещением.
    // Забирает массив у объекта-источника.
    ArrayPtr& operator=(ArrayPtr&& source)
    {
        // Если raw_ptr уже что-то хранит, это ошибка.
        // Можно молча удалить имеющееся, либо остановить выполнение assert'ом
#ifdef _DEBUG
        assert(this->raw_ptr_ == nullptr);
#else
        delete this->raw_ptr_;
#endif // !DEBUG_

        // Присваиваем указатель на массив
// Марина М.: std::exchange или свапнуть
//raw_ptr_ = source.raw_ptr_;
// Стираем указатель у источника
//source.raw_ptr_ = nullptr;
        raw_ptr_ = std::exchange(source.raw_ptr_, nullptr);
    }

    ~ArrayPtr()
    {
        delete[] raw_ptr_;
    }

    // Прекращает владением массивом в памяти, возвращает значение адреса массива
    // После вызова метода указатель на массив должен обнулиться
    [[nodiscard]] Type* Release() noexcept
    {
        Type* tmp_ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return tmp_ptr;
    }

    // Возвращает ссылку на элемент массива с индексом index
    Type& operator[](size_t index) noexcept
    {
        return *(raw_ptr_ + index);
    }

    // Возвращает константную ссылку на элемент массива с индексом index
    const Type& operator[](size_t index) const noexcept
    {
        Type* tmp_ptr = raw_ptr_ + index;
        return *tmp_ptr;
    }

    // Возвращает true, если указатель ненулевой, и false в противном случае
    explicit operator bool() const
    {
        return (raw_ptr_ != nullptr);
    }

    // Возвращает значение сырого указателя, хранящего адрес начала массива
    Type* Get() const noexcept
    {
        return raw_ptr_;
    }

    // Обменивается значениям указателя на массив с объектом other
    void swap(ArrayPtr& other) noexcept
    {
        std::swap(this->raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;

    // ArrayPtr не хранит размер массива -> следить нужно вручную
};
