#pragma once

#include <variant>

#include "owrt.hpp"

namespace dispatch
{
    template<typename T>
    concept Dispatch = requires {
        typename T::Variant;
    };

    template<typename... TVariants>
    struct DispatchGroup
    {
        using Variant = std::variant<TVariants...>;
    };

    template<typename T>
    concept WorldLike = requires {
        typename T::Num;
        typename T::ColorNum;

        typename T::Vec;
        typename T::Loc;
        typename T::Color;

        typename T::Ray;
    };

    template<
        class TNum,
        class TColorNum>
    struct WorldBase
    {
        using Self = WorldBase<TNum, TColorNum>;

        using Num = TNum;
        using ColorNum = TColorNum;
        
        using Vec = vmath::Vec3<Num>;
        using Loc = vmath::Loc3<Num>;
        using Color = color::Color3<ColorNum>;

        using Ray = vmath::Ray<Num>;
    };

    template<
        class TNum,
        class TColorNum,
        template<class> typename TConfig,
        template<class> typename TMatDispatch,
        template<class> typename TObjDispatch>
    struct World 
        : public WorldBase<TNum, TColorNum> 
    {
        using SelfBase = WorldBase<TNum, TColorNum>;
        using Self = World<TNum, TColorNum, TConfig, TMatDispatch, TObjDispatch>;

        using Config = TConfig<SelfBase>;

        using MatDispatch = TMatDispatch<Self>;
        using MatVar = MatDispatch::Variant;

        using ObjDispatch = TObjDispatch<Self>;
        using ObjVar = ObjDispatch::Variant;
    };
}