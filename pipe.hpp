#pragma once

#include <tuple>

namespace pipe {

template<typename Tp, typename... Ts>
struct statement;

namespace detail {

struct done_tag {};

template<typename... Ts>
struct type_list
{
    static_assert(sizeof...(Ts) > 0, "Cannot have an empty type_list");
  public:
    static constexpr std::size_t size = sizeof...(Ts);

    static constexpr std::size_t min_index = 0;
    static constexpr std::size_t max_index = size - 1;

    using tuple = std::tuple<Ts...>;

    template<std::size_t n>
    using get = typename std::tuple_element<n, tuple>::type;
    using front = get<min_index>;
    using back  = get<max_index>;

  private:
    template<std::size_t Offset, typename Seq>
    struct slice_helper_;

    template<std::size_t Offset, std::size_t... Is>
    struct slice_helper_<Offset, std::index_sequence<Is...>>
    {
        using type = type_list<get<Offset + Is>...>;
    };

    template<std::size_t Start, std::size_t Length>
    struct slice_helper
    {
        static_assert(Start >= min_index, "");
        static_assert(Start <= max_index, "");
        static_assert(Start + Length < size, "");

        using helper = slice_helper_<Start, std::make_index_sequence<Length>>;
        using type   = typename helper::type;
    };

  public:
    template<std::size_t Start = min_index, std::size_t End = max_index>
    using slice = typename slice_helper<Start, (End - Start)>::type;
};


template<typename Function>
struct prototype
  : prototype<decltype(&Function::operator())>{};

template<typename ClassType, typename R, typename... Args>
struct prototype<R(ClassType::*)(Args...) const>
{
    using type = R(ClassType::*)(Args...) const;
    using return_type = R;
    using args_types = type_list<Args...>;
};

template<typename ClassType, typename R, typename... Args>
struct prototype<R(ClassType::*)(Args...)>
{
    using type = R(ClassType::*)(Args...);
    using return_type = R;
    using args_types = type_list<Args...>;
};

template<typename R, typename... Args>
struct prototype<R(*)(Args..., ...)>
{
    using type = R(*)(Args..., ...);
    using return_type = R;
    using args_types = type_list<Args...>;
};

template<typename R, typename... Args>
struct prototype<R(*)(Args...)>
{
    using type = R(*)(Args...);
    using return_type = R;
    using args_types = type_list<Args...>;
};

template<typename R, typename... Args>
struct prototype<R(&)(Args...)>
{
    using type = R(&)(Args...);
    using return_type = R;
    using args_types = type_list<Args...>;
};

template<std::size_t Offset = 0, std::size_t... Is, typename Tuple>
constexpr auto tuple_subset(const Tuple& tpl, std::index_sequence<Is...>)
{
    return std::make_tuple(std::get<Is + Offset>(tpl)...);
}

template<typename Head, typename... Tail>
constexpr std::tuple<Tail...> tuple_pop(const std::tuple<Head, Tail...>& tpl)
{
    return tuple_subset<1>(tpl, std::make_index_sequence<sizeof...(Tail)>());
}

template<typename Tp, typename... Ts>
struct next_statement
{
    using type = statement<
        typename prototype<Tp>::return_type,
        Ts...
    >;
};

template<typename Tp, typename... Ts, std::enable_if_t<(sizeof...(Ts) > 0), bool> = true>
constexpr auto make_statement(Tp value, const std::tuple<Ts...>& tpl)
  -> statement<Tp, Ts...>
{
    return {value, tpl};
}

template<typename Tp>
constexpr auto make_statement(Tp value, std::tuple<>)
  -> statement<Tp>
{
    return {value};
}

} // namespace detail

template<typename Tp, typename... Ts>
struct statement
{
    using types     = detail::type_list<Ts...>;
    using next_type = typename detail::next_statement<Ts...>::type;

    Tp current_;
    std::tuple<Ts...> exprs_;
};

template<typename Tp>
struct statement<Tp>
{
    using types     = detail::type_list<Tp>;
    using next_type = Tp;

    Tp current_;
};

template<typename Fn, typename Tp, typename... Ts,
    std::enable_if_t<(sizeof...(Ts) > 0), bool> = true>
constexpr statement<Tp, Ts..., Fn> append(const statement<Tp, Ts...>& stmt, Fn fn)
{
    return {
        stmt.current_,
        std::tuple_cat(stmt.exprs_, std::make_tuple(fn))
    };
}

template<typename Fn, typename Tp>
constexpr statement<Tp, Fn> append(const statement<Tp>& stmt, Fn fn)
{
    return {
        stmt.current_,
        std::make_tuple(fn)
    };
}

template<typename Tp, typename... Ts, std::enable_if_t<(sizeof...(Ts) > 0), bool> = true>
constexpr auto exec(const statement<Tp, Ts...>& stmt)
{
    auto value = std::get<0>(stmt.exprs_)(stmt.current_);
    auto tpl   = detail::tuple_pop(stmt.exprs_);
    return exec(detail::make_statement(value, tpl));
}

template<typename Tp>
constexpr auto exec(const statement<Tp>& stmt)
{
    return stmt.current_;
}

template<typename Tp>
constexpr statement<Tp> with(Tp expr)
{
    return { expr };
}

template<typename Tp, typename... Ts, typename Fn>
constexpr auto operator|(const statement<Tp, Ts...>& stmt, Fn fn)
  -> statement<Tp, Ts..., Fn>
{
    return append(stmt, fn);
}

static detail::done_tag done{};

template<typename Tp, typename... Ts>
constexpr auto operator|(const statement<Tp, Ts...>& stmt, detail::done_tag)
{
    return exec(stmt);
}

}
