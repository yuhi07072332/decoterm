#include "style.hpp"

template <typename...Ts>
struct Tree{};


template <typename T>
struct is_tree: std::false_type {};

template <typename...Ts>
struct is_tree<Tree<Ts...>>: std::true_type {};


auto main() -> int {
    using namespace deco;
    int i = 16;
    const int ci = 32;
    float f = 3.14;
    int array[5] = { 1, 2, 3, 4, 5};

    auto l = detail::StyledValue(abs(bold), i);
    auto cl = detail::StyledValue(abs(bold), ci);
    decltype(auto) ls = l.value.get();
    decltype(auto) cls = cl.value.get();
    auto sl = detail::StyledValue(abs(bold), "helloworld");
    decltype(auto) sls = sl.value.get();
    auto al = detail::StyledValue(abs(bold), array);
    decltype(auto) als = al.value.get();

    auto r = detail::StyledValue(abs(bold), 64);
    auto cr = detail::StyledValue(abs(bold), std::move(ci));


    auto tree = Tree<int, double, Tree<float, int>, int>{};

    auto st = styled(bold, 31, i, styled(italic, f, "aaa"), ci);
    constexpr auto cest = styled(bold, 31, 16.5, "aaaa", styled(italic, "aa"))
}
