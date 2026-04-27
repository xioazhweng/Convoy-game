#pragma once

/**
 * @file LookUpTable.h
 * @brief Заголовочный файл с реализацией шаблонного класса LookUpTable
 * @details Этот файл содержит реализацию ассоциативного контейнера LookUpTable,
 * который реализует логику работы просматриваемой таблицы с полями `is_busy`.
 * (Вроде как) Удовлетворяет требованиям 
 * - BidirectionalIterator 
 * - AssociativeContainer 
 *      - AllocatorAwareContainer 
 *      - Container
 *
 * @see KeySpace
 * @see LookUpTableIterator
 * @see LookUpTable
 */

#include <cassert>
#include <memory>
#include <cstddef>
#include <iterator>
#include <utility>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <type_traits>

typedef enum {
    BUSY_FIRST,
    BUSY,
    FREE,
    SENTINEL, // SENTINEL: слот для несконструкированного значения, на него и будет указывать end()
    BUSY_LAST,
    BUSY_FIRST_LAST
} busy;

constexpr bool lut_has_value(busy s) noexcept {
    return s == BUSY_FIRST || s == BUSY || s == BUSY_LAST || s == BUSY_FIRST_LAST;
}

constexpr bool lut_is_sentinel(busy s) noexcept {
    return s == SENTINEL;
}

/**
 * @brief Структура для хранения элемента в просматриваемой таблице
 * @tparam T Тип хранимых значений. Должен быть деструктурируемым.
 * @tparam Allocator Тип аллокатора для управления памятью
 * @note Операции копирования удалены.
 */

template<typename T, typename KeyType>
struct KeySpace {
    using value_type = std::pair<const KeyType, T>;

    //Использование объединения позволяет осуществлять ручной контроль над временем жизни объекта
    union {
        value_type value;
    };
    
    busy is_busy = FREE;  
    KeySpace() noexcept : is_busy(FREE) {}
    //запрещаем копирование
    KeySpace(const KeySpace&) = delete;
    KeySpace& operator=(const KeySpace&) = delete;

    KeySpace(KeySpace&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>)
    : is_busy(other.is_busy) {
        if (lut_has_value(is_busy)) {
            new(&value) value_type(std::move(other.value));
            other.value.~value_type();
            other.is_busy = FREE;
        }
    }

    KeySpace& operator=(KeySpace&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>) {
        if (this != &other) {
            destroy_value();
            is_busy = other.is_busy;
            if (lut_has_value(is_busy)) {
                new(&value) value_type(std::move(other.value));
                other.value.~value_type();
                other.is_busy = FREE;
            }
        }
        return *this;
    }

    ~KeySpace() {
        destroy_value();
    }

    private:
        void destroy_value() noexcept {
            if (lut_has_value(is_busy)) {
                value.~value_type();
                is_busy = FREE;
            }
        }
};


/**
 * @brief Класс итератора для LookUpTable
 *
 * @details Реализует двунаправленный итератор (BidirectionalIterator), который пропускает
 * незанятые ячейки при обходе.
 *
 * @tparam T Тип хранимых значений
 * @tparam Allocator Тип аллокатора
 * @tparam is_const Флаг, указывающий, является ли итератор константным
 */
template<typename T, typename Allocator, typename KeyType, bool is_const>
class LookUpTableIterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<const KeyType, T>;
        using pointer = std::conditional_t<is_const, const value_type*, value_type*>;
        using reference = std::conditional_t<is_const, const value_type&, value_type&>;
        using iterator_category = std::bidirectional_iterator_tag;
        
        LookUpTableIterator() noexcept : ks_ptr(nullptr){};

        template<bool other_const>
        LookUpTableIterator(const LookUpTableIterator<T, Allocator, KeyType, other_const>& other) noexcept
        requires (is_const >= other_const)
        : ks_ptr(other.ks_ptr){};

        /**
        * @brief Оператор присваивания с преобразованием
        * @tparam other_const Константность исходного итератора
        * @param other Итератор для копирования
        * @return Ссылка на этот итератор
        * @details Позволяет присваивание от неконстантного к константному итератору
        */
        template<bool other_const>
        LookUpTableIterator& operator=(const LookUpTableIterator<T, Allocator, KeyType, other_const>& other) noexcept
        requires (is_const >= other_const) {
            ks_ptr = other.ks_ptr;
            return *this;
        }

        /**
        * @brief Оператор префиксного инкремента
        * @return Ссылка на этот итератор после инкремента
        * @details Продвигает итератор к следующей занятой ячейке
        */
        LookUpTableIterator& operator++() noexcept {
            if (ks_ptr == nullptr || lut_is_sentinel(ks_ptr->is_busy)) {
                return *this;
            }
            ++ks_ptr;
            while (ks_ptr->is_busy == FREE) {
                ++ks_ptr;
            }
            return *this;
        }

        /**
        * @brief Оператор постфиксного инкремента
        * @return Копия итератора до инкремента
        * @details Возвращает копию, затем продвигает к следующей занятой ячейке
        */
        LookUpTableIterator operator++(int) noexcept {
            LookUpTableIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        /**
        * @brief Оператор префиксного декремента
        * @return Ссылка на этот итератор после декремента
        * @details Перемещает итератор к предыдущей занятой ячейке
        */
        LookUpTableIterator& operator--() noexcept {
            if (ks_ptr == nullptr) {
                return *this;
            }

            if (ks_ptr->is_busy == BUSY_FIRST || 
                ks_ptr->is_busy == BUSY_FIRST_LAST) {
                return *this;
            }

            --ks_ptr;
            while (ks_ptr->is_busy == FREE) {
                --ks_ptr;
            }
            return *this;
        }

        /**
        * @brief Оператор постфиксного декремента
        * @return Копия итератора до декремента
        * @details Возвращает копию, затем перемещает к предыдущей занятой ячейке
        */
        LookUpTableIterator operator--(int) noexcept {
            LookUpTableIterator tmp = *this;
            --(*this);
            return tmp;
        }

        /**
        * @brief Оператор разыменования
        * @return Ссылка на значение в текущей позиции
        */
        reference operator*() const noexcept {
            return ks_ptr->value;
        }

        /**
        * @brief Оператор доступа к члену
        * @return Указатель на значение в текущей позиции
        */
        pointer operator->() const noexcept {
            return &ks_ptr->value;
        }

        /**
        * @brief Оператор сравнения на равенство
        * @tparam other_const Константность другого итератора
        * @param other Итератор для сравнения
        * @return true если оба итератора указывают на одну позицию, false в противном случае
        */
        template<bool other_const>
        bool operator==(const LookUpTableIterator<T, Allocator, KeyType, other_const>& other) const noexcept {
            return ks_ptr == other.ks_ptr;
        }

        /**
        * @brief Оператор сравнения на неравенство
        * @tparam other_const Константность другого итератора
        * @param other Итератор для сравнения
        * @return true если итераторы указывают на разные позиции, false в противном случае
        */
        template<bool other_const>
        bool operator!=(const LookUpTableIterator<T, Allocator, KeyType, other_const>& other) const noexcept {
            return !(*this == other);
        }

    private:
        using ks_pointer = std::conditional_t<is_const,
            const KeySpace<T, KeyType>*,
            KeySpace<T, KeyType>*>;

        ks_pointer ks_ptr;

        LookUpTableIterator(ks_pointer current) noexcept
        : ks_ptr(current) {}

        template<typename, typename, typename> friend class LookUpTable;
        template<typename U, typename A, typename k, bool c> friend class LookUpTableIterator;
};

/**
 * @brief Шаблонный класс LookUpTable - ассоциативный контейнер
 *
 * @details LookUpTable представляет собой ассоциативный контейнер, который реализует
 * логику работы просматриваемой таблицы с полями `is_busy`.
 *
 *
 * @tparam T Тип хранимых значений. Должен быть деструктурируемым и перемещаемо конструируемым.
 * @tparam Allocator Тип аллокатора для управления памятью (по умолчанию: std::allocator)
 *
 * @see KeySpace
 * @see LookUpTableIterator
 */
template <
    typename T,
    typename KeyType,
    typename Allocator = std::allocator<std::pair<const KeyType, T>>
>
class LookUpTable {
    public:
        using key_type = KeyType;
        using mapped_type = T;
        using value_type = std::pair<const key_type, mapped_type>;
        using allocator_type = Allocator;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
        using iterator = LookUpTableIterator<mapped_type, allocator_type, key_type, false>;
        using const_iterator = LookUpTableIterator<mapped_type, allocator_type, key_type, true>;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;
        using key_compare = std::less<key_type>;
        using value_compare = std::less<value_type>;


    private:
        // связываем keyspace с таблицей
        using ks_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<
            KeySpace<mapped_type, key_type>>;
        using ks_alloc_traits = std::allocator_traits<ks_allocator_type>;

        using ks_type = KeySpace<mapped_type, key_type>;

        ks_type* data_ = nullptr;
        size_type capacity_ = 0;
        size_type size_ = 0;
        size_type first_busy_ = 0;
        size_type last_busy_ = 0;

        allocator_type allocator_;
        ks_allocator_type ks_allocator_;

        ks_type* allocate_ks(size_type capacity) {
            const size_type n = (capacity == 0) ? 0 : (capacity + 1);
            return n ? ks_alloc_traits::allocate(ks_allocator_, n) : nullptr;
        }

        void deallocate_ks(ks_type* p, size_type capacity) noexcept {
            const size_type n = (capacity == 0) ? 0 : (capacity + 1);
            if (p && n) {
                ks_alloc_traits::deallocate(ks_allocator_, p, n);
            }
        }

        void destroy_ks_value(ks_type* p) noexcept {
            if (lut_has_value(p->is_busy)) {
                p->value.~value_type();
                p->is_busy = FREE;
            }
        }

        void destroy_ks_values_range(ks_type* first, ks_type* last) noexcept {
            for (; first != last; ++first) {
                destroy_ks_value(first);
            }
        }

        void destroy_ks_objects_range(ks_type* first, ks_type* last) noexcept {
            for (; first != last; ++first) {
                ks_alloc_traits::destroy(ks_allocator_, first);
            }
        }

        iterator make_iterator_at(size_type pos) noexcept {
            if (data_ == nullptr) {
                return iterator(nullptr);
            }
            return iterator(data_ + pos);
        }

        const_iterator make_const_iterator_at(size_type pos) const noexcept {
            if (data_ == nullptr) {
                return const_iterator(nullptr);
            }
            return const_iterator(data_ + pos);
        }

        void recompute_bounds_and_markers() noexcept {
            if (capacity_ == 0 || data_ == nullptr) {
                first_busy_ = last_busy_ = 0;
                return;
            }

            data_[capacity_].is_busy = SENTINEL;

            if (size_ == 0) {
                first_busy_ = last_busy_ = 0;
                return;
            }

            first_busy_ = 0;
            while (first_busy_ < capacity_ && !lut_has_value(data_[first_busy_].is_busy)) {
                ++first_busy_;
            }
            last_busy_ = capacity_ - 1;
            while (last_busy_ > first_busy_ && !lut_has_value(data_[last_busy_].is_busy)) {
                --last_busy_;
            }

            for (size_type i = 0; i < capacity_; ++i) {
                if (lut_has_value(data_[i].is_busy)) {
                    data_[i].is_busy = BUSY;
                }
            }

            if (first_busy_ == last_busy_) {
                data_[first_busy_].is_busy = BUSY_FIRST_LAST;
            } else {
                data_[first_busy_].is_busy = BUSY_FIRST;
                data_[last_busy_].is_busy = BUSY_LAST;
            }
        }

        size_type find_key_pos(const key_type& key) const noexcept {
            for (size_type i = 0; i < capacity_; ++i) {
                if (lut_has_value(data_[i].is_busy) && data_[i].value.first == key) {
                    return i;
                }
            }
            return capacity_;
        }

        size_type find_free_pos() const noexcept {
            for (size_type i = 0; i < capacity_; ++i) {
                if (data_[i].is_busy == FREE) {
                    return i;
                }
            }
            return capacity_;
        }

        void reorganize() {
            if (capacity_ == 0 || data_ == nullptr || size_ == 0) {
                first_busy_ = last_busy_ = 0;
                return;
            }

            size_type old_first = first_busy_;
            size_type old_last  = last_busy_;

            ks_type* new_data = nullptr;
            size_type constructed_slots = 0;
            size_type moved = 0;

            try {
                new_data = allocate_ks(capacity_);
                for (; constructed_slots < capacity_; ++constructed_slots) {
                    ks_alloc_traits::construct(ks_allocator_, &new_data[constructed_slots]);
                }
                
                ks_alloc_traits::construct(ks_allocator_, &new_data[capacity_]);
                new_data[capacity_].is_busy = SENTINEL;
                for (size_type i = 0; i < capacity_; ++i) {
                    if (lut_has_value(data_[i].is_busy)) {
                        if constexpr (
                            std::is_nothrow_move_constructible_v<value_type> ||
                            !std::is_copy_constructible_v<value_type>
                        ) {
                            ::new (static_cast<void*>(&new_data[moved].value))
                                value_type(std::move(data_[i].value));
                        } else {
                            ::new (static_cast<void*>(&new_data[moved].value))
                                value_type(data_[i].value);
                        }
                        new_data[moved].is_busy = BUSY;
                        ++moved;
                    }
                }

                
                destroy_ks_objects_range(data_, data_ + capacity_ + 1);
                deallocate_ks(data_, capacity_);

                data_ = new_data;
                // moved == size_
                recompute_bounds_and_markers();
            }
            catch (...) {
                if (new_data) {
                    destroy_ks_objects_range(new_data, new_data + constructed_slots + 1);
                    deallocate_ks(new_data, capacity_);
                }
                first_busy_ = old_first;
                last_busy_  = old_last;

                throw;
            }
 }

        void expand_capacity() {
            size_type new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            auto new_data = allocate_ks(new_capacity);

            size_type constructed = 0;
            try {
                for (; constructed < new_capacity; ++constructed) {
                    ks_alloc_traits::construct(ks_allocator_, &new_data[constructed]);
                }

                ks_alloc_traits::construct(ks_allocator_, &new_data[new_capacity]);
                new_data[new_capacity].is_busy = SENTINEL;
                for (size_type i = 0; i < capacity_; ++i) {
                    if (lut_has_value(data_[i].is_busy)) {
                        size_type new_pos = find_free_pos_in_range(new_data, new_capacity);
                        /*
                        if constexpr (std::is_nothrow_move_constructible_v<value_type> || !std::is_copy_constructible_v<value_type>) {
                            new (&new_data[new_pos].value) value_type(std::move(data_[i].value));
                        } else {
                            new (&new_data[new_pos].value) value_type(data_[i].value);
                        }*/
                        new (&new_data[new_pos].value) value_type(std::move_if_noexcept(data_[i].value));

                        new_data[new_pos].is_busy = BUSY;
                    }
                }
            } catch (...) {
                destroy_ks_objects_range(new_data, new_data + constructed + 1);
                deallocate_ks(new_data, new_capacity);
                throw;
            }

            if (data_) {
                destroy_ks_objects_range(data_, data_ + capacity_ + 1);
                deallocate_ks(data_, capacity_);
            }

            data_ = new_data;
            capacity_ = new_capacity;
            recompute_bounds_and_markers();
        }

        size_type find_free_pos_in_range(ks_type* arr, size_type cap) const noexcept {
            for (size_type i = 0; i < cap; ++i) {
                if (arr[i].is_busy == FREE) {
                    return i;
                }
            }
            return cap;
        }

    public:

        // =============== КОНСТРУКТОРЫ ===============

        /**
        * @brief Конструктор по умолчанию
        * 
        * Создает пустую таблицу с нулевой емкостью.
        * Использует аллокатор по умолчанию.
        */
        LookUpTable() noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
        : LookUpTable(allocator_type()) {}

        /**
        * @brief Конструктор с указанием аллокатора
        * 
        * Создает пустую таблицу с указанным аллокатором.
        * 
        * @param alloc Аллокатор для управления памятью
        */
        explicit LookUpTable(const allocator_type& alloc)
        : allocator_(alloc),
        ks_allocator_(alloc) {}

        /**
        * @brief Конструктор с указанием начальной емкости
        * 
        * Создает таблицу с заданной начальной емкостью.
        * 
        * @param capacity Начальная емкость таблицы
        * @param alloc Аллокатор для управления памятью
        */
        explicit LookUpTable(size_type capacity, const allocator_type& alloc = allocator_type())
        : allocator_(alloc),
        ks_allocator_(alloc),
        capacity_(capacity) {
            if (capacity_ > 0) {
                data_ = allocate_ks(capacity_);
                for (size_type i = 0; i < capacity_; ++i) {
                    ks_alloc_traits::construct(ks_allocator_, &data_[i]);
                }
                ks_alloc_traits::construct(ks_allocator_, &data_[capacity_]);
                data_[capacity_].is_busy = SENTINEL;
            }
        }

        /**
        * @brief Конструктор копирования
        * @param other LookUpTable для копирования
        * @param alloc Аллокатор для использования в управлении памятью
        * @details Создает глубокую копию всех элементов из other.
        *          Новая таблица имеет такую же емкость и размер, как и other.
        * @throw Любое исключение, которое может быть выброшено конструктором value_type или аллокатором
        */
        LookUpTable(const LookUpTable& other, const allocator_type& alloc = allocator_type())
        : allocator_(alloc),
        ks_allocator_(alloc),
        capacity_(other.capacity_),
        size_(other.size_),
        first_busy_(other.first_busy_),
        last_busy_(other.last_busy_) {
            if (capacity_ > 0) {
                data_ = allocate_ks(capacity_);
                size_type constructed = 0;
                try {
                    for (; constructed < capacity_; ++constructed) {
                        ks_alloc_traits::construct(ks_allocator_, &data_[constructed]);
                    }
                    ks_alloc_traits::construct(ks_allocator_, &data_[capacity_]);
                    data_[capacity_].is_busy = SENTINEL;

                    for (size_type i = 0; i < capacity_; ++i) {
                        if (lut_has_value(other.data_[i].is_busy)) {
                            new (&data_[i].value) value_type(other.data_[i].value);
                            data_[i].is_busy = BUSY;
                        }
                    }

                    recompute_bounds_and_markers();
                } catch (...) {
                    destroy_ks_objects_range(data_, data_ + constructed + 1);
                    deallocate_ks(data_, capacity_);
                    data_ = nullptr;
                    capacity_ = size_ = first_busy_ = last_busy_ = 0;
                    throw;
                }
            }
        }

        /**
        * @brief Конструктор перемещения
        * @param other LookUpTable для перемещения
        * @param alloc Аллокатор для использования в управлении памятью
        * @details Если аллокаторы равны, выполняет перемещение за постоянное время.
        *          В противном случае элементы перемещаются индивидуально.
        *          После перемещения other находится в допустимом, но неопределенном состоянии.
        * @note noexcept если аллокатор не генерирует исключения при перемещающем конструировании
        * @throw Любое исключение, которое может быть выброшено при конструировании элементов (когда аллокаторы разные)
        */
        LookUpTable(LookUpTable&& other, const allocator_type& alloc = allocator_type())
        noexcept(std::allocator_traits<allocator_type>::is_always_equal::value &&
                 std::is_nothrow_move_constructible_v<allocator_type>)
        : allocator_(alloc),
        ks_allocator_(alloc) {

            if constexpr (std::allocator_traits<allocator_type>::is_always_equal::value) {
                data_ = other.data_;
                capacity_ = other.capacity_;
                size_ = other.size_;
                first_busy_ = other.first_busy_;
                last_busy_ = other.last_busy_;

                other.data_ = nullptr;
                other.capacity_ = other.size_ = other.first_busy_ = other.last_busy_ = 0;
            } else {
                if (alloc == other.get_allocator()) {
                    data_ = other.data_;
                    capacity_ = other.capacity_;
                    size_ = other.size_;
                    first_busy_ = other.first_busy_;
                    last_busy_ = other.last_busy_;

                    other.data_ = nullptr;
                    other.capacity_ = other.size_ = other.first_busy_ = other.last_busy_ = 0;
                } else {
                    capacity_ = other.capacity_;
                    size_ = other.size_;
                    first_busy_ = other.first_busy_;
                    last_busy_ = other.last_busy_;

                    if (capacity_ > 0) {
                        data_ = allocate_ks(capacity_);
                        size_type constructed = 0;
                        try {
                            for (; constructed < capacity_; ++constructed) {
                                ks_alloc_traits::construct(ks_allocator_, &data_[constructed]);
                            }
                            ks_alloc_traits::construct(ks_allocator_, &data_[capacity_]);
                            data_[capacity_].is_busy = SENTINEL;

                            for (size_type i = 0; i < capacity_; ++i) {
                                if (lut_has_value(other.data_[i].is_busy)) {
                                    new (&data_[i].value) value_type(std::move(other.data_[i].value));
                                    data_[i].is_busy = BUSY;

                                    other.data_[i].value.~value_type();
                                    other.data_[i].is_busy = FREE;
                                }
                            }

                            recompute_bounds_and_markers();
                        } catch (...) {
                            destroy_ks_objects_range(data_, data_ + constructed + 1);
                            deallocate_ks(data_, capacity_);
                            data_ = nullptr;
                            capacity_ = size_ = first_busy_ = last_busy_ = 0;
                            throw;
                        }
                    }
                }
            }
        }

        /**
        * @brief Конструктор диапазона
        * @tparam It Тип входного итератора
        * @param first Итератор на первый элемент в диапазоне
        * @param last Итератор за последним элементом в диапазоне
        * @param alloc Аллокатор для использования в управлении памятью
        * @details Конструирует таблицу из диапазона пар ключ-значение [first, last).
        *          Каждый элемент должен быть преобразуемым в std::pair<const key_type, T>.
        * @throw Любое исключение, которое может быть выброшено методом insert
        */
        template<std::input_iterator It>
        LookUpTable(It first, It last, const allocator_type& alloc = allocator_type())
        : LookUpTable(alloc) {
            LookUpTable temp(alloc);
            for (; first != last; ++first) {
                temp.insert(*first);
            }
            swap(temp);
        }

    // LookUpTable<int> table = {{"один", 1}, {"два", 2}, {"три", 3}};
        /**
        * @brief Конструктор списка инициализации
        * @param init Список инициализации пар ключ-значение
        * @param alloc Аллокатор для использования в управлении памятью
        * @details Конструирует таблицу из списка инициализации.
        * @throw Любое исключение, которое может быть выброшено методом insert
        */
        LookUpTable(std::initializer_list<std::pair<const key_type, T>> init,
                    const allocator_type& alloc = allocator_type())
        : LookUpTable(init.begin(), init.end(), alloc) {}

        /**
        * @brief Деструктор
        * @details Уничтожает все элементы и освобождает память
        */
        ~LookUpTable() {
            clear();
            if (data_) {
                destroy_ks_objects_range(data_, data_ + capacity_ + 1);
                deallocate_ks(data_, capacity_);
            }
        }
        
        /**
        * @brief Оператор присваивания копированием
        * @param other LookUpTable для копирования
        * @return Ссылка на эту таблицу
        * @details Заменяет содержимое копией элементов other.
        *          Самоприсваивание обрабатывается корректно.
        * @throw Любое исключение, которое может быть выброшено конструктором копирования
        */
        LookUpTable& operator=(const LookUpTable& other) {
            if (this != &other) {
                LookUpTable temp(other, get_allocator());
                swap(temp);
            }
            return *this;
        }

        /**
        * @brief Оператор перемещяющего присваивания
        * @param other LookUpTable для перемещения
        * @return Ссылка на эту таблицу
        * @details Заменяет содержимое перемещением из other.
        *          После операции other находится в допустимом, но неопределенном состоянии.
        * @note noexcept если аллокатор всегда равен и не генерирует исключения при перемещающем присваивании
        */
        LookUpTable& operator=(LookUpTable&& other)
        noexcept(std::allocator_traits<allocator_type>::is_always_equal::value &&
                std::is_nothrow_move_assignable_v<allocator_type>) {
            if (this == &other) {
                return *this;
            }
            LookUpTable tmp(std::move(other));  
            std::swap(data_, tmp.data_);
            std::swap(size_, tmp.size_);
            std::swap(capacity_, tmp.capacity_);
            std::swap(first_busy_, tmp.first_busy_);
            std::swap(last_busy_, tmp.last_busy_);
            return *this;
        }

        /**
        * @brief Оператор присваивания списком инициализации
        * @param init Список инициализации пар ключ-значение
        * @return Ссылка на эту таблицу
        * @details Заменяет содержимое элементами из списка инициализации
        * @throw Любое исключение, которое может быть выброшено методом insert
        */
        LookUpTable& operator=(std::initializer_list<std::pair<const key_type, T>> init) {
            LookUpTable temp(init.begin(), init.end(), get_allocator());
            swap(temp);
            return *this;
        }

        /**
        * @brief Заменяет содержимое элементами из диапазона
        * @tparam InputIterator Тип входного итератора
        * @param first Итератор на первый элемент в диапазоне
        * @param last Итератор за последним элементом в диапазоне
        * @details Очищает таблицу и вставляет все элементы из [first, last)
        * @throw Любое исключение, которое может быть выброшено методом insert
        */
        template<std::input_iterator InputIterator>
        void assign(InputIterator first, InputIterator last) {
            LookUpTable temp(get_allocator());
            for (; first != last; ++first) {
                temp.insert(*first);
            }
            swap(temp);
        }
        /**
        * @brief Заменяет содержимое элементами из списка инициализации
        * @param ilist Список инициализации пар ключ-значение
        * @details Очищает таблицу и вставляет все элементы из списка
        * @throw Любое исключение, которое может быть выброшено методом insert
        */
        void assign(std::initializer_list<std::pair<const key_type, T>> ilist) {
            LookUpTable temp(ilist.begin(), ilist.end(), get_allocator());
            swap(temp);
        }

        /**
        * @brief Возвращает аллокатор, связанный с контейнером
        * @return Копию объекта аллокатора
        */
        allocator_type get_allocator() const noexcept {
            return allocator_;
        }

        /**
        * @brief Возвращает итератор на первый элемент
        * @return Итератор на первый занятый элемент, или end() если пусто
        * @details Возвращенный итератор автоматически пропускает незанятые ячейки
        */
        iterator begin() noexcept {
            if (capacity_ == 0 || data_ == nullptr || size_ == 0) {
                return end();
            }
            return iterator(data_ + first_busy_);
        }

        /**
         * @brief Возвращает константный итератор на первый элемент
         * @return Константный итератор на первый занятый элемент, или cend() если пусто
         */
        const_iterator begin() const noexcept {
            if (capacity_ == 0 || data_ == nullptr || size_ == 0) {
                return end();
            }
            return const_iterator(data_ + first_busy_);
        }

        /**
         * @brief Возвращает константный итератор на первый элемент
         * @return Константный итератор на первый занятый элемент, или cend() если пусто
         */
        const_iterator cbegin() const noexcept {
            return begin();
        }

        /**
         * @brief Возвращает итератор на элемент после последнего
         * @return Итератор на элемент после последнего
         */
        iterator end() noexcept {
            if (capacity_ == 0 || data_ == nullptr) {
                return iterator(nullptr);
            }
            return iterator(data_ + capacity_);
        }   

        /**
         * @brief Возвращает константный итератор на элемент после последнего
         * @return Константный итератор на элемент после последнего
         */
        const_iterator end() const noexcept {
            if (capacity_ == 0 || data_ == nullptr) {
                return const_iterator(nullptr);
            }
            return const_iterator(data_ + capacity_);
        }   

        /**
         * @brief Возвращает константный итератор на элемент после последнего
         * @return Константный итератор на элемент после последнего
         */
        const_iterator cend() const noexcept {
            return end();
        }

        /**
        * @brief Возвращает количество элементов в таблице
        * 
        * @return size_type Количество элементов в таблице
        */
        size_type size() const noexcept { return size_; }

        /**
        * @brief Возвращает максимально возможный размер таблицы
        * 
        * @return size_type Максимальный размер таблицы
        */
        size_type max_size() const noexcept {
            return ks_alloc_traits::max_size(ks_allocator_);
        }

        /**
        * @brief Проверяет, пуста ли таблица
        * 
        * @return bool true если таблица пуста, false в противном случае
        */
        bool empty() const noexcept { return size_ == 0; }

        /**
        * @brief Возвращает текущую емкость таблицы
        * 
        * @return size_type Текущая емкость таблицы
        */
        size_type capacity() const noexcept { return capacity_; }


        /**
         * @brief Вставляет элемент в таблицу
         * 
         * Вставляет пару ключ-значение в таблицу.
         * Если ключ уже существует, вставка не выполняется.
         * 
         * @param value Пара ключ-значение для вставки
         * @return pair<iterator, bool> Итератор на существующий/вставленный элемент и флаг успеха
         */
        std::pair<iterator, bool> insert(const std::pair<const key_type, T>& value) {
            return emplace(value);
        }

        /**
         * @brief Вставляет элемент в таблицу с перемещением
         *
         * Вставляет пару ключ-значение с использованием семантики перемещения.
         *
         * @param value Пара ключ-значение для вставки
         * @return pair<iterator, bool> Итератор на вставленный элемент и флаг успеха
         */
        std::pair<iterator, bool> insert(std::pair<const key_type, T>&& value) {
            return emplace(std::move(value));
        }

        /**
         * @brief Конструирует и вставляет элемент на месте
         * 
         * @param args Аргументы для конструктора `value_type` (std::pair<const key_type, mapped_type>)
         * @return pair<iterator, bool> Итератор на вставленный элемент и флаг успеха
         * @throw Любое исключение, которое может быть выброшено конструктором value_type или при расширении емкости
         */
        template<typename... Args>
        std::pair<iterator, bool> emplace(Args&&... args) {
            value_type v(std::forward<Args>(args)...);
            const key_type& key = v.first;
            size_type pos = find_key_pos(key);
            if (pos < capacity_) {
                return {make_iterator_at(pos), false};
            }
            pos = find_free_pos();
            if (pos == capacity_) {
                expand_capacity();
                pos = find_free_pos();
            }
            try {
                new (&data_[pos].value) value_type(std::move(v));
            } catch (...) {
                throw;
            }
            data_[pos].is_busy = BUSY;
            ++size_;
            recompute_bounds_and_markers();
            return {make_iterator_at(pos), true};
        }

        /**
        * @brief Находит элемент по ключу
        * 
        * @param key Ключ для поиска
        * @return iterator Итератор на найденный элемент или end() если элемент не найден
        */
        iterator find(const key_type & key) {
            size_type pos = find_key_pos(key);
            if (pos < capacity_) {
                return make_iterator_at(pos);
            }
            return end();
        }

        /**
         * @brief Находит элемент по ключу (константная версия)
         *
         * Выполняет линейный поиск элемента с указанным ключом в константной таблице.
         *
         * @param key Ключ для поиска
         * @return const_iterator Константный итератор на найденный элемент или end() если элемент не найден
         */
        const_iterator find(const key_type & key) const {
            size_type pos = find_key_pos(key);
            if (pos < capacity_) {
                return make_const_iterator_at(pos);
            }
            return end();
        }

        /**
        * @brief Подсчитывает количество элементов с заданным ключом
        * 
        * В ассоциативном контейнере ключи уникальны, поэтому возвращает 0 или 1.
        * 
        * @param key Ключ для подсчета
        * @return size_type Количество элементов с данным ключом (0 или 1)
        */
        size_type count(const key_type & key) const {
            return find_key_pos(key) < capacity_ ? 1 : 0;
        }

        /**
        * @brief Удаляет элемент по ключу
        * 
        * Находит и удаляет элемент с указанным ключом.
        * При необходимости выполняет реорганизацию таблицы.
        * 
        * @param key Ключ элемента для удаления
        * @return size_type Количество удаленных элементов (0 или 1)
        * @throw Любое исключение, которое может быть выброшено деструктором value_type
        */
        size_type erase(const key_type& key) noexcept {
            size_type pos = find_key_pos(key);
            if (pos >= capacity_) {
                return 0;
            }
            destroy_ks_value(&data_[pos]);
            --size_;
            recompute_bounds_and_markers();
            return 1;
        }

        /**
        * @brief Удаляет элемент по итератору
        * 
        * Удаляет элемент, на который указывает итератор.
        * 
        * @param position Итератор на элемент для удаления
        * @return iterator Итератор на следующий элемент
        * @throw Любое исключение, которое может быть выброшено деструктором value_type
        */
        iterator erase(const_iterator position) noexcept {
            if (position == end()) {
                return end();
            }
            size_type pos = position.ks_ptr - data_;
            auto temp = std::move(data_[pos]);
            destroy_ks_value(&data_[pos]);
            --size_;
            recompute_bounds_and_markers();
            iterator next = make_iterator_at(pos);
            ++next;
            return next;
        }

        /**
        * @brief Удаляет диапазон элементов
        * 
        * Удаляет все элементы в диапазоне [first, last).
        * 
        * @param first Итератор на начало диапазона
        * @param last Итератор на конец диапазона
        * @return iterator Итератор на элемент после последнего удаленного
        * @throw Любое исключение, которое может быть выброшено деструктором value_type
        */
        iterator erase(const_iterator first, const_iterator last) noexcept {
            iterator result = end();
            while (first != last) {
                result = erase(first++);
            }
            return result;
        }

        /**
        * @brief Очищает таблицу
        * 
        * Удаляет все элементы из таблицы, освобождая память.
        * Размер таблицы становится равным нулю, но емкость сохраняется.
        */
        void clear() noexcept {
            for (size_type i = 0; i < capacity_; ++i) {
                if (lut_has_value(data_[i].is_busy)) {
                    destroy_ks_value(&data_[i]);
                }
            }
            size_ = 0;
            first_busy_ = last_busy_ = 0;
        }

        /**
        * @brief Доступ к элементу по ключу с проверкой границ
        * 
        * Возвращает ссылку на элемент с указанным ключом.
        * Бросает исключение std::out_of_range если ключ не найден.
        * 
        * @param key Ключ для доступа
        * @return T& Ссылка на найденный элемент
        * @throws std::out_of_range если ключ не найден в таблице
        */
        T& at(const key_type & key) {
            size_type pos = find_key_pos(key);
            if (pos >= capacity_) {
                throw std::out_of_range("Key is not found");
            }
            return data_[pos].value.second;
        }

        /**
        * @brief Доступ к элементу по ключу с проверкой границ (константная версия)
        * 
        * Возвращает константную ссылку на элемент с указанным ключом.
        * Бросает исключение std::out_of_range если ключ не найден.
        * 
        * @param key Ключ для доступа
        * @return const T& Константная ссылка на найденный элемент
        * @throws std::out_of_range если ключ не найден в таблице
        */
        const T& at(const key_type & key) const {
            size_type pos = find_key_pos(key);
            if (pos >= capacity_) {
                throw std::out_of_range("Key is not found");
            }
            return data_[pos].value.second;
        }

        
        /**
        * @brief Доступ к элементу или вставка с заданным ключом
        * @param key Ключ для поиска или вставки
        * @return Ссылка на значение, связанное с ключом
        * @details Если ключ существует, возвращает ссылку на его значение.
        *          Если ключ не существует, вставляет значение по умолчанию
        *          и возвращает ссылку на него.
        */
        T& operator[](const key_type& key) {
            size_type pos = find_key_pos(key);
            if (pos < capacity_) {
                return data_[pos].value.second;
            }
            emplace(key, T{});
            pos = find_key_pos(key);
            return data_[pos].value.second;
        }

        /**
         * @brief Доступ к элементу или вставка с заданным ключом (версия для rvalue)
         * @param key Ключ для поиска или вставки (перемещается, если происходит вставка)
         * @return Ссылка на значение, связанное с ключом
         */
        T& operator[](key_type&& key) {
            size_type pos = find_key_pos(key);
            if (pos < capacity_) {
                return data_[pos].value.second;
            }
            auto res = emplace(std::move(key), T{});
            return res.first->second;
        }


        /**
        * @brief Оператор сравнения на равенство
        * @param other Таблица для сравнения
        * @return true если обе таблицы содержат одинаковые пары ключ-значение
        */
        bool operator==(const LookUpTable& other) const {
            if (size_ != other.size_) return false;
            return std::equal(begin(), end(), other.begin(), other.end());
        }

        /**
        * @brief Оператор сравнения на неравенство
        * @param other Таблица для сравнения
        * @return true если таблицы не равны
        */
        bool operator!=(const LookUpTable& other) const {
            return !(*this == other);
        }


        /**
        * @brief Обменивается содержимым с другой таблицей
        * @param other Таблица для обмена
        * @details Обменивается содержимым этой таблицы с other.
        *          Все итераторы и ссылки остаются действительными, но теперь
        *          относятся к обмененному контейнеру.
        * @note noexcept если аллокатор всегда равен и не генерирует исключения при обмене
        */
        void swap(LookUpTable& other) noexcept(
            std::allocator_traits<allocator_type>::is_always_equal::value &&
            std::is_nothrow_swappable_v<allocator_type>) {

            using std::swap;
            swap(data_, other.data_);
            swap(capacity_, other.capacity_);
            swap(size_, other.size_);
            swap(first_busy_, other.first_busy_);
            swap(last_busy_, other.last_busy_);

            if constexpr (!std::allocator_traits<allocator_type>::is_always_equal::value) {
                swap(allocator_, other.allocator_);
                swap(ks_allocator_, other.ks_allocator_);
            }
        }


        /**
        * @brief Резервирует память для указанного количества элементов
        *
        * Увеличивает емкость таблицы до указанного значения, если оно больше текущей емкости.
        * Не влияет на размер таблицы.
        *
        * @param new_capacity - новая емкость таблицы
        * @throw Любое исключение, которое может быть выброшено конструктором value_type или аллокатором
        */
        void reserve(size_type new_capacity) {
            if (new_capacity > capacity_) {
                auto new_data = allocate_ks(new_capacity);

                size_type constructed = 0;
                try {
                    for (; constructed < new_capacity; ++constructed) {
                        ks_alloc_traits::construct(ks_allocator_, &new_data[constructed]);
                    }
                    ks_alloc_traits::construct(ks_allocator_, &new_data[new_capacity]);
                    new_data[new_capacity].is_busy = SENTINEL;

                    for (size_type i = 0; i < capacity_; ++i) {
                        if (lut_has_value(data_[i].is_busy)) {
                            size_type new_pos = find_free_pos_in_range(new_data, new_capacity);
                            /*
                            if constexpr (std::is_nothrow_move_constructible_v<value_type> || !std::is_copy_constructible_v<value_type>) {
                                new (&new_data[new_pos].value) value_type(std::move(data_[i].value));
                            } else {
                                new (&new_data[new_pos].value) value_type(data_[i].value);
                            }
                                */
                            new (&new_data[new_pos].value) value_type(std::move_if_noexcept(data_[i].value));
                            new_data[new_pos].is_busy = BUSY;
                        }
                    }
                } catch (...) {
                    destroy_ks_objects_range(new_data, new_data + constructed + 1);
                    deallocate_ks(new_data, new_capacity);
                    throw;
                }

                if (data_) {
                    destroy_ks_objects_range(data_, data_ + capacity_ + 1);
                    deallocate_ks(data_, capacity_);
                }

                data_ = new_data;
                capacity_ = new_capacity;
                recompute_bounds_and_markers();
            }
        }

        
    };

    /**
    * @brief Специализация функции обмена для LookUpTable
    * @tparam T Тип хранимых значений
    * @tparam Allocator Тип аллокатора
    * @param a Первая таблица для обмена
    * @param b Вторая таблица для обмена
    * @relates LookUpTable
    * @details Обменивается содержимым a и b. Эквивалентно a.swap(b).
    */
    template<typename T, typename KeyType, typename Allocator>
    void swap(LookUpTable<T, KeyType, Allocator>& a, LookUpTable<T, KeyType, Allocator>& b) noexcept(noexcept(a.swap(b))) {
        a.swap(b);
    }


    static_assert(std::bidirectional_iterator<LookUpTableIterator<int, std::allocator<int>, int, false>>);
    static_assert(std::bidirectional_iterator<LookUpTableIterator<int, std::allocator<int>, int, true>>);

