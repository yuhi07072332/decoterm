# Style stack 重构方案

## 背景问题

当前 Style stack 依赖全局 `OutputState`。每次输出 `Style`、`AbsoluteStyle`、`pop`、`reset`，包括 `operator<<(std::ostream&, ...)` 和 `std::format`，都会隐式修改全局状态。

这会带来两个主要问题：

- `OutputState` 语义不清晰：它看起来像终端输出状态，但 `ostream` 和 `std::format` 的目标不一定是 stdout，也不一定是终端。
- Style stack 使用场景有限：嵌套 style 是局部输出需求，不应该靠全局状态维护。

## 重构原则

- 管理状态的功能应放进显式创建的局部对象中，而不是全局 `OutputState`。
- Style stack、base style、style output on/off、color fallback 都应属于局部输出上下文。
- `std::format` / `std::print` 不应隐式读写全局 style 状态。

## 推荐方向：`StyleOut`

主推一个绑定 `std::ostream` 的局部输出包装类：

```cpp
deco::StyleOut sout(std::cout);

sout << fg(blue)
     << "lorem ipsum "
     << (bold | color(black, red))
     << "IMPORTANT TEXT"
     << pop
     << "back to normal"
     << reset;
```

`StyleOut` 负责解释 style token，并维护局部状态：

- 当前 style
- style stack
- base style
- 是否输出 escape sequence
- 是否启用 color fallback

普通 `std::ostream` 仍然只接收最终渲染结果，不参与 style 状态管理。

## 普通 Style 输出语义

`Style` / `AbsoluteStyle` 应保持纯值类型：

```cpp
std::cout << fg(red) << "error" << reset;
std::format("{}text{}", fg(red), reset);
```

这些调用只输出 escape sequence，不 push style、不 pop style、不检查 stdout、不触发 fallback。

建议 0.1 前移除或隐藏这些全局 API：

- `output::set_style_stack(bool)`
- `output::style_stack_enabled()`
- `output::current_style()`
- 全局 style stack 的 push/pop/reset 语义

`pop` 可以只在 `StyleOut` 中有意义；普通 `ostream` / `formatter` 可以不支持 `pop`，避免产生误导。

## `Terminal` 的职责

`Terminal` 应只负责终端能力探测和平台初始化：

- Windows VT mode setup
- stdout 是否为 terminal
- color support detection

`Terminal` 不应直接修改全局输出状态。推荐用法是把探测结果显式传给 `StyleOut`：

```cpp
deco::Terminal term;

deco::StyleOut sout(std::cout, deco::StyleOutOption {
    .style_enabled = term.is_stdout_terminal(),
    .color_fallback = term.color_support() != deco::ColorSupport::TrueColor,
});
```

## `std::format` / `std::print` 处理

不建议 0.1 实现完整的 `StyleFmt` 来复刻 `std::format` / `std::print` / fmtlib API。原因是 API 面很大，而且 style token 在 format 字符串中是否影响后续状态需要额外定义，容易引入复杂语义。

更推荐给 `StyleOut` 提供轻量 convenience：

```cpp
sout << fg(red);
sout.print("error: {} at {}", message, location);
sout << reset;
```

`StyleOut::print` / `println` 只负责普通文本格式化，可以内部使用 `std::format_to(std::ostreambuf_iterator<char>)` 避免中间字符串。style 状态仍然通过 `operator<<` 改变。

暂不鼓励这种 API 作为 0.1 主路径：

```cpp
sfmt.format("{}error{}", fg(red), reset);
```

如果未来确实需要 `StyleFmt`，建议放到独立 header 中，以窄接口逐步实验，而不是承诺完整替代 `std::format` 或 fmtlib。

## 0.1 建议范围

- 将普通 `Style` / `AbsoluteStyle` 输出改为无状态 escape generation。
- 移除或内部化全局 Style stack。
- 新增 `StyleOut`，承载 stack、base style、style output on/off、color fallback。
- 保留 `reset` 作为无状态 reset escape；`pop` 只在 `StyleOut` 中解释。
- `Terminal` 改为能力查询对象，不再隐式影响所有输出。
- 可选增加 `StyleOut::print` / `println`，不要急于实现完整 `StyleFmt`。

## 暂缓事项

- 完整 `StyleFmt` API。
- fmtlib alternative。
- 全局 base style。
- 全局 style stack。
- 在 `std::format` 中让 style token 修改共享状态。
