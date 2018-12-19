# Testing string substitutions.

global var1 = value1
global var2 = value2
global var3 = value3
global list1 = [list John A Smith]
global list2 = [list Jane A Smith]

global string = "${var1}${var2}${var3}"
print $string

global string = "$var1$var2$var3"
print $string

global string = "Hello [$list1 join][$list2 join]"
print $string

global string = "This is a variable \$name.\n\tAnd a \"backslash\" \\\n\t\$var1 = $var1"
print $string

print "\x48\x65\x6c\x6c\x6f!"
