# texc

A texc expander program written in pure c (currently only supports windows). Inspired by [espanso](https://github.com/espanso/espanso).


# Features
- Fast and lightweight
- [CLI](/docs/info.md#command-line-interface) Support
- [HTTP Rest Api](/docs/info.md#http-rest-api)

# Building

```
cmake -S . -B build
cmake --build build -DCMAKE_BUILD_TYPE=Release
```

# Usage

## Starting
To start the texc app run,
```
texc --port {port}
```
or
```
texc server --port {port}
```

## Adding a Text Expansion

To add a text-expansion do,
```
texc add {text} {expanded}
```

Example:
```
texc add hello world
```
Now if we type 'hello' it will be replaced by 'world'

## Removing a Text Expansion
To remove a existing text-expansion do,
```
texc remove {text}
```

For example if we want to remove the 'hello' text-expansion we do,
```
texc remove hello
```

## Closing the Server
Do the following to close the server
```
texc close
```

## Other Info

See [info.md](/docs/info.md) for more info.