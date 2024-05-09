# Command line interface

The command line interface is documented in the `-h, --help` flags. CLI uses the http rest api to communicate between texc server and texc command. 

# Http Rest Api

1. [/version](./api/version.md)
2. [/add](./api/add.md)
3. [/remove](./api/remove.md)
4. [/list](./api/list.md)
5. [/close](./api/close.md)
6. [/config](./api/config.md)

# Text Expansion System

## Enable

A text-expansion is only active if the enable attribute is set to true. Can be set using `config` sub command.

## Undo

Undo is possible by pressing backspace after the expansion. But undo is disabled if non-undoable tags are used in either `text` section or `expand` section

## Tags

See [tags.md](tags.md) for all the tags available
<hr>

texc allows the usage of special tags inside `text` or `expand` sections.

For example we can do,
```
texc add "hello<enter>" world
```
Which will only replace hello after enter.<hr>

```
texc add hello "word<enter>"
```
Which will type word and then enter<hr>

```
texc add "<tcase>hello</tcase>" word
```
Which will toggle case sensitivity to be off.  

## How to use `<` and `>`
To use the characters `<` and `>` in the text-expansion do `<<` and `>>` which will be converted into `<` and `>`. So for example `<<enter>>` will type `<enter>` instead of pressing enter 

## Difference between `"a"` and `<a>`
In the `expand` section we can use tags which contain lower case alpha numberic characters.  

```
texc add hello "<a>"
```
But this is not same as,
```
texc add hello a
```  
When we use in the first case the 'a' key will be affected by modifiers such as caps lock, but in the second case 'a' key will be typed as it is, with modifier keys not affecting it.