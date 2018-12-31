# Testing string substitutions.

import stdlib

global var1 = value1
global var2 = value2
global var3 = value3
global list1 = [list John A Smith]
global list2 = [list Jane A Smith]

global string = "${var1}${var2}${var3}"
if {$string <> "value1value2value3"} then {
  print "$string <> value1value2value3"
  return 1
}

global string = "$var1$var2$var3"
if {$string <> "value1value2value3"} then {
  print "$string <> value1value2value3"
  return 1
}

global string = "Hello [$list1 join][$list2 join]"
if {$string <> "Hello John A SmithJane A Smith"} then {
  print "$string <> Hello John A SmithJane A Smith"
  return 1
}

global string = "This is a variable \$name.\n\tAnd a \"backslash\" \\\n\t\$var1 = $var1"
print $string

global string = "\x48\x65\x6c\x6c\x6f!"
if {$string <> "Hello!"} then {
  print "$string <> Hello!"
  return 1
}
