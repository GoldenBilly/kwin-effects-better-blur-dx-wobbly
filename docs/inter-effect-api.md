# Better Blur DX inter-effect mesh API v1

This fork adds one provider-owned rendering path for Better Wobbly Windows while
keeping the regular Better Blur DX behavior and settings aligned with upstream.
The `v1` label belongs to the inter-effect API only; it is not a Better Wobbly
Windows release version.


## Effect selection

This fork is intended to be the active Better Blur DX implementation. The original
`xarblu/kwin-effects-better-blur-dx` effect may remain installed, but it should be
disabled in KWin's Desktop Effects while this fork is enabled. Running both effects
at the same time can make them compete for the same blur work and may cause visual or
rendering conflicts.

## Frame flow

1. Better Wobbly Windows publishes a `MeshProvider` pointer in `RequestRole`
   during `prePaintWindow()`.
2. Better Blur DX validates API version 1, the provider structure size and the
   mesh callback.
3. Better Blur DX creates or updates its own blur cache and invokes the callback
   with the cache geometry and current render scale.
4. Better Wobbly Windows returns deformed vertices plus cache-space source
   coordinates.
5. Better Blur DX performs the final composite with its own shaders and writes
   `ProviderResult::Rendered` to `ResultRole`.
6. When the provider is unavailable or fails technically, Better Blur DX writes
   `ProviderResult::Declined`, and Better Wobbly Windows draws its standalone
   fallback.
7. When Better Blur DX intentionally excludes the window through its matching or
   window-type policy, it writes `ProviderResult::DeclinedByPolicy`
   (`declined_by_policy`). Better Wobbly Windows must leave the window unblurred
   and must not replace the decision with fallback blur.

## Publishing a request

```cpp
BetterBlurDxApi::MeshProvider provider;
provider.apiVersion = BetterBlurDxApi::Version;
provider.structSize = sizeof(provider);
provider.context = reinterpret_cast<quintptr>(this);
provider.buildMesh = &buildMesh;

window->setData(BetterBlurDxApi::ResultRole, QVariant());
window->setData(BetterBlurDxApi::RequestRole,
                BetterBlurDxApi::makeRequest(&provider, true, true));
```

Clear both roles when the handoff ends or before the provider object is
destroyed:

```cpp
window->setData(BetterBlurDxApi::RequestRole, QVariant());
window->setData(BetterBlurDxApi::ResultRole, QVariant());
```

## Deciding whether fallback is needed

```cpp
const QVariant result = window->data(BetterBlurDxApi::ResultRole);
const bool rendered = BetterBlurDxApi::providerRendered(result);
const bool declinedByPolicy = BetterBlurDxApi::providerDeclinedByPolicy(result);

if (!rendered && !declinedByPolicy) {
    drawStandaloneFallback();
}
```

`ResultRole` is intentionally short-lived. Better Blur DX clears it before and
after each nested `drawWindow()` chain.
