## The Cutlet utility language

Inspired by [Tcl](https://www.tcl.tk/), *Cutlet* is designed to be a small and simple utility language to be used by other programs and projects. Where Tcl started as a basic _Tool Command Language_ and has, more or less , become a general programming language over the years, Cutlet's design is meant to be a simple utility language only.

### Features

**Sand-boxing**

The entire global environment can be removed, replaced, redefined or overridden by the use of sandboxes. This effectively makes Cutlet a general parser in which to implement the task that is needed from it.

**Simple Syntax**

1. One line one command.
   ```
   This is command one
   This is command two
   ```
2. Each part of the command is separated by spaces.
   ```
   This command has 5 words
   command arg1 arg2 arg3
   ```
3. Variables.

   A name starting with the `$` character gets replaced with a variable's value.
   ```tcl
   global customer_name = "John Smith"
   print $customer_name

   local counter = 10
   print We have $counter items.
   ```
4. Command substitutions.

   Square brackets, `[]`, gets replaced with the return value from the command in
   them.
   ```tcl
   local mylist = [list value1 value2 value3]
   print [$mylist join .]
     -> value1.value2.value3
   ```
5. Strings and substitutions.

   Strings are denoted with the quoting characters "" and ''. Variable and command substitution can be preformed in strings.
   ```tcl
   "Hello $client, your order [get_order $client] is ready\n"
   'Hello $client, your order [get_order $client] is ready\n'
   ```
6. Blocks.

  Blocks are denoted with curly brackets, {}. They can be used as literal strings and the only part of the language that can span multiple lines.
  ```tcl
  print {Hello World}

  def my_command {
    command one
    command two
  }
  ```
7. Comments

  Lines starting with the *#* character are comment lines and are ignored by Cutlet.
  ```tcl
  # This is a comment.
  print Hello # This is not a comment
  ```

**Simple Implementation**

Cutlet as a language doesn't define any language constructs with the exception
of variable, command and string substitutions. Things like looping, conditional
branching, object orientation, procedures and functions and so on, are defined
by libraries or by the encapsulating program. Even setting variable values is not defined by Cutlet and is implementation specific.

### Uses for Cutlet

**Configuration files**

XML is a popular format for configuration files. Although a text editable file, XML is very verbose with a lot of punctuation and be hard to understand. Here's a simple example of Xfce4 configuration.
```xml
<?xml version="1.0" encoding="UTF-8"?>
<channel name="xsettings" version="1.0">
  <property name="Net" type="empty">
    <property name="ThemeName" type="string" value="Greybird"/>
    <property name="IconThemeName" type="string" value="elementary-xfce-dark"/>
    <property name="DoubleClickTime" type="empty"/>
    <property name="DoubleClickDistance" type="empty"/>
    <property name="DndDragThreshold" type="empty"/>
    <property name="CursorBlink" type="empty"/>
    <property name="CursorBlinkTime" type="empty"/>
    <property name="SoundThemeName" type="empty"/>
    <property name="EnableEventSounds" type="bool" value="false"/>
    <property name="EnableInputFeedbackSounds" type="bool" value="false"/>
    <property name="FallbackIconTheme" type="empty"/>
  </property>
</channel>
```
Cutlet can be used to create a slimmer configuration that is easier to understand and modifiy.
```tcl
# Xfce4 XSettings
Net {
  ThemeName                 Greybird
  IconThemeName             elementary-xfce-dark
  DoubleClickTime           default
  DoubleClickDistance       default
  DndDragThreshold          default
  CursorBlink               default
  CursorBlinkTime           default
  SoundThemeName            default
  EnableEventSounds         false
  EnableInputFeedbackSounds false
  FallbackIconTheme         default
}
```
Making some of Cutlets scripting capabilities available, complicated configurations can be handled well as in this http server example.
```tcl
# Include other configuration files.
include "/etc/httpd/conf.d/*.conf"

# The domains we host.
set virtual_hosts [list www.host1.com www.host2.com]

# Configure each host.
$virtual_host foreach host {
  virtual $host {
    # Default configuration for each virtual host.
    directory "/var/www/$host"
  }
}
```
**Intermediate files**

Just like configuration files, Cutlet can be used to quickly create file formats for intermediate files like project files.

**Embedded scripting**

Like most scripting languages, it can be embedded into software to allow custom extending after release.
