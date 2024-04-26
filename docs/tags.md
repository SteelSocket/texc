# Tags
Tags are special syntax that modify how `text` gets matched or are used to press non character keys such as enter, tab, backspace, etc.  
Tags are written in the style of xml tags, i.e `<tag></tag>` or `<tag>`  

# `Text` Section Tags

## `<enter>` Tag
- Is used to detect the user pressing enter
- Is standalone tag only

## `<tab>` Tag
- Is used to detect the user pressing tab
- Is standalone tag only

## `<tcase>` Tag
- Toggles the case sensitivity for the text contained in it.
- Must only contain text
- Example: `<tcase>hello</tcase>` will match 'hello' regardless of case  
  

# `Expand` Section Tags
Expand tags written in container format will hold the given key and release it when all the contents of the tags are typed.  
Example: `<shift><a></shift>` will hold shift while typing a

## `<character>` Tags
- They are single character tags which must be lower case alpha numeric values.
- Can be used as standalone tag or as a container
- See [info.md](./info.md/#difference-between-a-and-a) for more information
- Example: `<a>`, `<1>`, `<z>`, `...`

## `<enter>` Tag
- Can be used as standalone tag or as a container
- Presses and Releases the enter key

## `<backspace>` Tag
- Can be used as standalone tag or as a container
- Presses and Releases the backspace key

## `<tab>` Tag
- Can be used as standalone tag or as a container
- Presses and Releases the tab key

## `<shift>` Tag
- Can be used as standalone tag or as a container
- Presses and Releases the shift key

## `<ctrl>` Tag
- Can be used as standalone tag or as a container
- Presses and Releases the ctrl key
