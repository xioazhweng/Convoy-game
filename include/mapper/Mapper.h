#pragma once

template<typename From, typename To, typename FromWrap = From, typename ToWrap = To>
class Mapper {
public:
    using FromType = From;
    using ToType = To;
    using FromWrapType = FromWrap;
    using ToWrapType = ToWrap;

    virtual ~Mapper() = default;
    [[nodiscard]] virtual ToWrap map_to(const From&) const = 0;
    [[nodiscard]] virtual FromWrap map_from(const To&) const = 0;
};


/**
 * @brief Вспомогательная функция для сборки «составного» id из двух частей (16+16 бит)
 *
 */
inline unsigned get_id(unsigned a, unsigned b) {
    return (static_cast<unsigned>(a) << 16) | b;
}
