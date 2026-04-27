#pragma once
#include "Mapper.h"

#include <map>
#include <memory>
#include <string>
#include <typeindex>

template<typename From, typename To, typename FromWrap = From, typename ToWrap = To, typename Key = std::string>
class SubtypeMapper: public Mapper<From, To, FromWrap, ToWrap> {
public:
    using KeyType = Key;

    [[nodiscard]] virtual Key get_key() const = 0;
    [[nodiscard]] virtual std::type_index get_type() const = 0;
};

template<typename From, typename To, typename FromWrap = From, typename ToWrap = To,
         typename Key = std::string>
class PolymorphicMapper: public Mapper<From, To, FromWrap, ToWrap> {
public:

    using Supertype = Mapper<From, To, FromWrap, ToWrap>;
    using Subtype = SubtypeMapper<From, To, FromWrap, ToWrap, Key>;

    void add_sub_mapper(const Subtype& subMapper) {
        concreteMappersByKey_.emplace(subMapper.get_key(), std::cref(subMapper));
        concreteMappersByType_.emplace(subMapper.get_type(), std::cref(subMapper));
    }

    [[nodiscard]] virtual Key get_key(const To&) const = 0;

    [[nodiscard]] ToWrap map_to(const From& from) const override {
        std::type_index type = typeid(from);
        return concreteMappersByType_.at(type).get().map_to(from);
    }

    [[nodiscard]] FromWrap map_from(const To& to) const override {
        auto key = get_key(to);
        return concreteMappersByKey_.at(key).get().map_from(to);
    }

private:

    std::map<Key, std::reference_wrapper<const Subtype>> concreteMappersByKey_;
    std::map<std::type_index, std::reference_wrapper<const Subtype>> concreteMappersByType_;
};

template<typename M>
concept IsSubmapper = requires(M v) {
    requires std::derived_from<M, SubtypeMapper<typename M::FromType, typename M::ToType,
                                                typename M::FromWrapType, typename M::ToWrapType>>;
};

template<typename SuperFrom, typename SuperFromWrap, IsSubmapper SubMapper>
class SubtypeMapperAdapter final : public SubtypeMapper<SuperFrom, typename SubMapper::ToType,
                                                         SuperFromWrap, typename SubMapper::ToWrapType,
                                                         typename SubMapper::KeyType> {

    SubMapper sub_;

    using Super = SubtypeMapper<SuperFrom, typename SubMapper::ToType,
                                                        SuperFromWrap, typename SubMapper::ToWrapType,
                                                        typename SubMapper::KeyType>;

public:

    template<typename ... Args>
    explicit SubtypeMapperAdapter(Args&& ... args)
        requires std::constructible_from<SubMapper, Args&&...>:
        sub_(std::forward<Args>(args)...)
    {}

    [[nodiscard]] Super::ToWrapType map_to(const Super::FromType& from) const override {
        return sub_.map_to(dynamic_cast<const SubMapper::FromType&>(from));
    }

    [[nodiscard]] SuperFromWrap map_from(const Super::ToType& to) const override {
        auto sub_result = sub_.map_from(to);
        return std::unique_ptr<SuperFrom>(static_cast<SuperFrom*>(sub_result.release()));
    }

    [[nodiscard]] Super::KeyType get_key() const override {
        return sub_.get_key();
    }

    [[nodiscard]] std::type_index get_type() const override {
        return sub_.get_type();
    }
};
