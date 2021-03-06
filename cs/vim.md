- [Move](#move)
  - [行内](#行内)
  - [跨行](#跨行)
  - [大范围移动](#大范围移动)
- [Edit](#edit)
  - [visual mode](#visual-mode)
  - [大小写](#大小写)
- [Match](#match)
  - [Replacement](#replacement)
- [File](#file)
- [Vim 多文件编辑](#vim-多文件编辑)
  - [Buffer](#buffer)
  - [tab](#tab)
    - [打开与关闭](#打开与关闭)
    - [标签跳转](#标签跳转)
  - [window](#window)
    - [打开关闭命令](#打开关闭命令)
    - [打开关闭快捷键](#打开关闭快捷键)
    - [切换窗口](#切换窗口)
    - [移动窗口](#移动窗口)
    - [调整大小](#调整大小)
  - [vi与shell切换](#vi与shell切换)
- [寄存器](#寄存器)
  - [寄存器分类](#寄存器分类)

- `*`可以查找当前光标下的word(完全符合),`g*`则部分符合,以`#`代替`*`表示向后(上)找.
- typing two backquotes \`\` returns you to the position where you issued the G command
- `[[`  跳转到代码块的开头去(但要求代码块中'{'必须单独占一行)
- `''`  跳转到光标上次停靠的地方, 是两个, 而不是一个

| command   | description                      |
| ---       | ---                              |
| :set var? | 查看var 的值, vim 中没有get 方法 |
| `:<C-f>`  | command history                  |
| :f[ile]   | 查看当前文件的全路径             |
| :e[dit]   | reload the file                  |


# Move
[vim 入坑指南(三)— motion 进阶](https://vimzijun.net/2016/08/06/vim-motion/)

## 行内
| command | movement         |
| ---     | ---              |
| e or E  | To end of word   |
| w or W  | Forward by word  |
| b or B  | Backward by word |

f (find) 和 t (till) 的功能都是在行内 向右 做单字符查找,并移动光标,只不过 f 是把光标挪到字符上, t 则是把光标挪到字符的左
边 (till 的意思).如果想要 向左 做查找,使用 F 和 T 便好了.

- 移至第一个 r 处: fr
- 移至第二个 r 处: 2fr
- 移至第一个 r 的左边: tr
- 移至第二个 r 的左边: 2tr

和 d 配合使用
假设光标在 relativenumber 词首

- 删除 relative, 按 dtn (delete till n)
- 删除 relative, 按 d2fe (delete find 2 e)

## 跨行
| command | movement                            |
| ---     | ---                                 |
| +       | To first character of next line     |
| -       | To first character of previous line |

## 大范围移动
窗口内

| command | movement             |
| ---     | ---                  |
| H       | Home, 跳到屏幕上端   |
| L       | Last, 跳到屏幕下端   |
| M       | Middle, 跳到屏幕中间 |

文件范围内

| command  | movement             |
| ---      | ---                  |
| gg       | 移光标至文件第一行   |
| G        | 移光标至文件最后一行 |
| ctrl + f | Forward by window    |
| ctrl + b | Backward by window   |
| %        | 跳到配对的括号       |
| nG or :n | 到第n行              |

# Edit
缩进

- `>`  增加缩进, `x>` 表示增加以下 x 行的缩进
- `<`  减少缩进, `x<` 表示减少以下 x 行的缩进

- Vim高亮显示的那个特殊的`^M`, 是Ctrl+V
- transposing two letters `xp`, 也就是一个`x`, 然后再`p`
- `:retab`: will replace all sequences of `<Tab>` with new strings of white-space using the new tabstop
  (e.g. :set tabstop=2) value given, but all tabs inside of strings can be modified (e.g. in a C program, you should use
  \t to avoid this)!

## visual mode
在Vim里面有三种激活可视模式,并选择一块区域的方法,他们分别是:

| 模式类型 | 激活方式 | 选择效果             | status  |
| ---      | ---      | ---                  | ---     |
| 字符文本 | v(小写)  | 逐个字符选择文本     | VISUAL  |
| 行文本   | V(大写)  | 逐行选择文本         | V-LINE  |
| 块文本   | `<C-v>`  | 按照块的方式选择文本 | V-BLOCK |

visual mode 下

- o: 可视模式的时候, 默认情况下只可以控制右下角的端点,使用o按键来在左上角和右下角之间进行切换.
- G: 选择当前位置到文章结尾
- $: 选择当前位置到当前行尾

- 列删除
  1. 按下组合键"CTRL+v" 进入可视块模式,选取这一列操作多少行
  1. 按下d 即可删除被选中的整块

- 列插入
  1. 按下组合键"CTRL+v" 进入可视块模式,选取这一列操作多少行
  1. 按下shift+i
  1. 输入要插入的内容(这里要注意只会第一行会显示插入的内容, 其他行需要按下esc 之后才会显示, 新手应注意)
  1. 按ESC,之后就会看到插入的效果

Editing action

| Editing action                               | Command |
|----------------------------------------------|---------|
| Delete line and substitute text              | S       |
| Overstrike existing characters with new text | R       |
| Toggle case                                  | ~       |
| Repeat last action                           | .       |

Edit commands

| Text object                         | Change     | Delete     | Copy       |
|-------------------------------------|------------|------------|------------|
| One word                            | cw         | dw         | yw         |
| Two words, not counting punctuation | 2cW or c2W | 2dW or d2W | 2yW or y2W |
| Three words back                    | 3cb or c3b | 3db or d3b | 3yb or y3b |
| One line                            | cc         | dd         | yy or Y    |
| To end of line                      | c$ or C    | d$ or D    | y$         |
| To beginning of line                | c0         | d0         | y0         |
| Single character                    | r          | x or X     | yl or yh   |
| Five characters                     | 5s         | 5x         | 5yl        |

More editing commands

| Change     | Delete     | Copy       | From cursor to...         |
|------------|------------|------------|---------------------------|
| cH         | dH         | yH         | Top of screen             |
| cL         | dL         | yL         | Bottom of screen          |
| c+         | d+         | y+         | Next line                 |
| 2c)        | 2d)        | 2y)        | Second sentence following |
| c{         | d{         | y{         | Previous paragraph        |
| c/ pattern | d/ pattern | y/ pattern | Pattern                   |
| cn         | dn         | yn         | Next pattern              |
| cG         | dG         | yG         | End of file               |
| c13G       | d13G       | y13G       | Line number 13            |

## 大小写
| Command | Explanation                          |
|---------|--------------------------------------|
| ?~      | 将光标下的字母改变大小写             |
| 3~      | 将光标位置开始的3个字母改变其大小写  |
| g~~     | 改变当前行字母的大小写               |
| U       | 将可视模式下选择的字母全改成大写字母 |
| u       | 将可视模式下选择的字母全改成小写     |
| gUU     | 将当前行的字母改成大写               |
| 3gUU    | 将从光标开始到下面3行字母改成大写    |
| guu     | 将当前行的字母全改成小写             |
| gUw     | 将光标下的单词改成大写               |
| guw     | 将光标下的单词改成小写               |

- `\u or \l`: Causes the next character in the replacement string to be changed to uppercase or lowercase, respectively.
- `\U or \L and \e or \E`: all following characters are converted to uppercase or lowercase until the end of the replacement string or until \e or \E is reached.
- `:%s/Fortran/\U&/`

# Match
删除包含特定字符的行(全局匹配): `g/pattern/d`

删除不包含指定字符的行: `v/pattern/d` 或者`g!/pattern/d`

## Replacement
- `%s/a/b/gc`: 末尾的c代表确认的意思, 在进行每一次替换时会询问是否替换
- `&` is replaced by the entire text matched by the search pattern when used in a replacement
  - `:%s/Yazstremski/&,clar/`  #the result is `Yazstremski,clar`
  - `:1,10s/.*/(&)/` #surrend each line from line 1 to line 10 with parentheses
- 在正则表达式中使用 `\(` 和 `\)` 符号括起正则表达式,即可在后面使用`\1`,`\2`等变量来访问 `\(` 和 `\)` 之间的内容

[vim 正则表达式](http://blog.csdn.net/yyt8yyt8/article/details/7567455)

表示内容的元字符

| 模式   | 含义                                              |
| ---    | ---                                               |
| .      | 匹配任意字符                                      |
| [abc]  | 匹配方括号中的任意一个字符.可以使用-表示字符范围  |
| [^abc] | 表示匹配除方括号中字符之外的任意字符.             |
| \d     | 匹配阿拉伯数字,等同于[0-9].                       |
| \D     | 匹配阿拉伯数字之外的任意字符,等同于[^0-9]         |
| \x     | 匹配十六进制数字,等同于[0-9A-Fa-f].               |
| \X     | 匹配十六进制数字之外的任意字符,等同于[^0-9A-Fa-f] |
| \w     | 匹配单词字母,等同于`[0-9A-Za-z_]`                 |
| \W     | 匹配单词字母之外的任意字符,等同于`[^0-9A-Za-z_]`  |
| \t     | 匹配<TAB>字符.                                    |
| \s     | 匹配空白字符,等同于[ \t].                         |
| \S     | 匹配非空白字符,等同于[^ \t].                      |
| \a     | 所有的字母字符. 等同于[a-zA-Z]                    |
| \l     | 小写字母 [a-z]                                    |
| \L     | 非小写字母 [^a-z]                                 |
| \u     | 大写字母 [A-Z]                                    |
| \U     | 非大写字母 [^A-Z]                                 |

一般的正则表达式中表示或的方法:`(my name)|(your name)`

表示数量的元字符

| 模式   | 含义                                                                                       |
| ---    | ---                                                                                        |
| *      | 匹配0-任意个                                                                               |
| \+     | 匹配1-任意个                                                                               |
| \?     | 匹配0-1个                                                                                  |
| \{n,m} | 匹配n-m个                                                                                  |
| \{n}   | 匹配n个                                                                                    |
| \{n,}  | 匹配n-任意个                                                                               |
| \{,m}  | 匹配0-m个                                                                                  |
| \_.    | 匹配包含换行在内的所有字符                                                                 |
| \{-}   | 表示前一个字符可出现零次或多次,但在整个正则表达式可以匹配成功的前提下,匹配的字符数越少越好 |
| \=     | 匹配一个可有可无的项                                                                       |
| \_s    | 匹配空格或断行                                                                             |

懒惰模式

- `\{-n,m}` 与`\{n,m}`一样,尽可能少次数地重复
- `\{-}` 匹配它前面的项一次或0次, 尽可能地少

**表示转义和位置的元字符**

| 模式 | 含义          |
| ---  | ---           |
| `\*` | 匹配 `*` 字符 |
| `\.` | 匹配 `.` 字符 |
| `\/` | 匹配 `/` 字符 |
| `\\` | 匹配 `\` 字符 |
| `\[` | 匹配 `[` 字符 |
| `$`  | 匹配行尾      |
| `^`  | 匹配行首      |
| `\<` | 匹配单词词首  |
| `\>` | 匹配单词词尾  |

# File
- `:w` write the buffer to the file
- `:230,$w newfile` #saves line 230 through the end of the file as a newfile
- `:230,$w >> newfile`
- `:q` quit
- `:w!` force write
- `:r` read
- `:$r filename` #place the content of `filename` at the end of the current session
- `:e filename` # editing another file, use `Ctrl+^` to swith to another file
- `:! command` execute the command in the shell
- `vi + filename` open file at last line
- `vi +/pattern filename` open file at the first occurrence of pattern
- `vi -R filename` readmode
- `vim -b file` edit binary file or we can use `:set binary`

**gain root permission without leaving vim**  
Often when I edit some files which require root permission (e.g. the files under /etc) I forget run vim with sudo.  
To force a save use the following command  
`:w !sudo tee %`  
It will prompt you for your password

# Vim 多文件编辑
<img src="./pics/vim/tabs-windows-buffers.png" alt="tabs windows buffers" width="70%"/>

A buffer is the in-memory text of a file. A window is a viewport on a buffer. A tab page is a collection of windows.

## [Buffer](https://harttle.land/2015/11/17/vim-buffer.html)
A buffer is an area of Vim's memory used to hold text read from a file. In addition, an empty buffer with no associated
file can be created to allow the entry of text. –vim.wikia

打开与关闭

不带任何参数打开多个文件便可以把它们都放入缓冲区(Buffer):
```bash
vim a.txt b.txt
```

缓冲区跳转

缓冲区之间跳转最常用的方式便是 Ctrl+^(不需要按下Shift)来切换当前缓冲区和上一个缓冲区. 另外,还提供了很多跳转命令:

- `:ls, :buffers`: 列出所有缓冲区
- `:bn[ext]`: 下一个缓冲区
- `:bp[revious]`: 上一个缓冲区
- `:b {number, expression}`: 跳转到指定缓冲区
	- `:b2`将会跳转到编号为2的缓冲区,如果你正在用:ls列出缓冲区,这时只需要输入编号回车即可.
	- :`b exa`将会跳转到最匹配exa的文件名,比如example.html,模糊匹配打开文件正是Vim缓冲区的强大之处.

然后按下`:b <Tab>`便可看到Vim提供的备选文件列表了, 按下`<Tab>`选择下一个,按下回车打开当前文件.

有了fzf, 可以使用fzf 的buffer 模式来搜索跳转

**recovering from buffer**  
`$ex -r` or `$vi -r`  
you will get a list of any files that the system has saved  
`$vi -r file` #to recover the file

## [tab](https://harttle.land/2015/11/12/vim-tabpage.html)
### 打开与关闭
使用-p参数来用多个标签页启动Vim:
```bash
vim -p main.cpp my-oj-toolkit.h /private/etc/hosts
```

| command           | description                                                         |
| ---               | ---                                                                 |
| :tabe[dit] {file} | edit specified file in a new tab                                    |
| :tabf[ind] {file} | open a new tab with filename given, searching the 'path' to find it |
| :tabc[lose]       | close current tab                                                   |
| :tabc[lose] {i}   | close i-th tab                                                      |
| :tabo[nly]        | close all other tabs (show only the current tab)                    |

### 标签跳转

| command   | description        |
| ---       | ---                |
| :tabn     | go to next tab     |
| :tabp     | go to previous tab |
| :tabfirst | go to first tab    |
| :tablast  | go to last tab     |

在正常模式(normal)下,还可以使用快捷键:

| command | description             |
| ---     | ---                     |
| gt      | go to next tab          |
| gT      | go to previous tab      |
| {i}gt   | go to tab in position i |

## [window](https://harttle.land/2015/11/14/vim-window.html)
分屏打开多个文件

使用-O参数可以让Vim以分屏的方式打开多个文件:
```bash
vim -O main.cpp my-oj-toolkit.h
```
使用小写的-o可以水平分屏.

### 打开关闭命令

在进入Vim后,可以使用这些命令来打开/关闭窗口:

| command          | description              |
| ---              | ---                      |
| :sp[lit] {file}  | 水平分屏                 |
| :new {file}      | 水平分屏                 |
| :sv[iew] {file}  | 水平分屏,以只读方式打开 |
| :vs[plit] {file} | 垂直分屏                 |
| :clo[se]         | 关闭当前窗口             |

上述命令中,如未指定file则打开当前文件.

### 打开关闭快捷键
| command  | description                  |
| ---      | ---                          |
| Ctrl+w s | 水平分割当前窗口             |
| Ctrl+w v | 垂直分割当前窗口             |
| Ctrl+w q | 关闭当前窗口                 |
| Ctrl+w n | 打开一个新窗口(空文件)       |
| Ctrl+w o | 关闭出当前窗口之外的所有窗口 |
| Ctrl+w T | 当前窗口移动到新标签页       |

### 切换窗口
切换窗口的快捷键就是Ctrl+w前缀 + hjkl:

| command  | description    |
| ---      | ---            |
| Ctrl+w h | 切换到左边窗口 |
| Ctrl+w j | 切换到下边窗口 |
| Ctrl+w k | 切换到上边窗口 |
| Ctrl+w l | 切换到右边窗口 |
| Ctrl+w w | 遍历切换窗口   |

还有t切换到最上方的窗口,b切换到最下方的窗口.

### 移动窗口
分屏后还可以把当前窗口向任何方向移动,只需要将上述快捷键中的hjkl大写:

| command  | description      |
| ---      | ---              |
| Ctrl+w H | 向左移动当前窗口 |
| Ctrl+w J | 向下移动当前窗口 |
| Ctrl+w K | 向上移动当前窗口 |
| Ctrl+w L | 向右移动当前窗口 |

### 调整大小
调整窗口大小的快捷键仍然有Ctrl+W前缀:

| command  | description  |
| ---      | ---          |
| Ctrl+w + | 增加窗口高度 |
| Ctrl+w - | 减小窗口高度 |
| Ctrl+w = | 统一窗口高度 |

横向调整

- :vertical res(ize) num 指定当前窗口为num列
- :vertical res(ize) +num 把当前窗口增加num列
- :vertical res(ize) -num 把当前窗口减少num列

## vi与shell切换
- :shell 可以在不关闭vi的情况下切换到shell命令行
- exit 再从shell回到vi

# 寄存器
通过下面命令可以查看所有寄存器中的内容,也可以只查看指定寄存器的内容(将寄存器名称作为参数)
```vim
:reg [register_name]
```

一般来讲,可以用 "{register}y 来拷贝到 {register} 中, 用 "{register}p 来粘贴 {register} 中的内容.例如: "ayy 可以拷贝当前
行到寄存器 a 中,而 "ap 则可以粘贴寄存器 a 中的内容.

[如何将 Vim 剪贴板里面的东西粘贴到 Vim 之外的地方?](https://www.zhihu.com/question/19863631/answer/442180294)

根据平台不同,要分两种情况.先用下面命令确定你属于哪一种,
```bash
vim --version | grep clipboard
```
如果找到的是负号开头的-clipboard,说明你的vim不支持系统剪切板,需要先重新安装vim

- Linux系统,`sudo apt install vim-gtk`

目前over ssh 还没实验成功

- [How can I copy text to the system clipboard from Vim?](
https://vi.stackexchange.com/questions/84/how-can-i-copy-text-to-the-system-clipboard-from-vim)
- [vim + COPY + mac over SSH](https://stackoverflow.com/questions/10694516/vim-copy-mac-over-ssh)

## [寄存器分类](https://harttle.land/2016/07/25/vim-registers.html)
- 匿名寄存器 `""`: 默认unnamed寄存器,最近一次"d","c","s","x","y"复制,删除,修改内容
- 编号寄存器 `"0` 到 `"9`
  - `"0`: 最近一次"y"复制内容
  - `"1`: 最近一次"d","c","s","x"删除,修改内容
  - `"2`: 上一次"d","c","s","x"删除,修改内容
  - `"3`: 上上次"d","c","s","x"删除,修改内容
  - ...
  - `"9`: [1-9]数字以此类推
- 小删除寄存器 `"-`: 少于一行的"d","c","x"删除内容
- `"a-z`: 26个字母寄存器,供用户指定使用,比如"ay就是复制到"a寄存器
- 3个只读寄存器
  - `".`: 只读寄存器, 上次 insert 模式中插入的字符串
  - `":`: 只读寄存器, 上次命令模式下键入的命令
  - `"%`: 只读寄存器, 从当前Vim 的工作目录到该文件的路径
- Buffer 交替文件寄存器 `"#`: 存储着当前 Vim 窗口(Window)的交替文件.交替文件(alternate file)是指 Buffer 中的上一个文件,
  可通过 Ctrl+^ 来切换交替文件与当前文件
- 表达式寄存器 `"=`: 当我 `们`键入 "= 后光标会移动到命令行,此时我们可以输入任何 Vim 脚本的表达式. 例如 3+2,按下回车并且p
  则会得到 5
- 选择和拖放寄存器, 包括`"*`, `"+`, 和`"~`,这三个寄存器的行为是和 GUI 相关的.
  - `"*` 和 "+ `在` Mac 和 Windows 中,都是指系统剪切板(clipboard),例如`"*yy` 即可复制当前行到剪切板. 其他程序中复制的内容
  也会被存储到这两个寄存器中. 在 X11 系统中(绝大多数带有桌面环境的 Linux 发行版),二者是有区别的:
    - `"*` 指 X11 中的 PRIMARY 选区,即鼠标选中区域.在桌面系统中可按鼠标中键粘贴.
    - `"+` 指 X11 中的 CLIPBOARD 选区,即系统剪切板.在桌面系统中可按 Ctrl+V 粘贴.
  - 有文本拖拽到 Vim 时,被拖拽的文本被存储在 "~ 中.Vim 默认的行为是将 "~ 中内容插入到光标所在位置.
- 搜索模式寄存器 "/: 存储上一次搜索的关键词

最后`set clipboard=unnamed`就是把默认无名寄存器`""` 和系统剪贴板也关联上. 就是用y也可以备份到系统剪贴板.缺点是破坏了默认
寄存器""的本地性.因为p操作也会被等同于"+p处理,粘贴的是"+寄存器的内容, 粘贴的时候""默认寄存器内容就会被覆盖. 表现出来的就
是复制一次,到任意vim窗口都可以粘贴.但这个特性恰恰是很多人想要的.

