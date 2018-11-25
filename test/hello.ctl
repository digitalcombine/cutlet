
def join {*args} {
  # Return the list as a joined string.
  return [$args join]
}

# Testing variables and results
global name "Ron Wills"

# Testing strings with variable
print "Hello ${name}!"
print Hello [join Josie A Vis]
print "Hello [join Ross Bready]"

print
print "Guess what" # This isn't a comment.
print

$library.path append "../libs/.libs"

import oo

class test_class {

  property the_name

  method new {a_name} {
    local the_name = $a_name
  }

  method hello {} {
    print "Hello to $the_name from the oo system"
    $self goodbye
  }

  method goodbye {
    print "See ya later $the_name"
  }
}

print "New class created"

global myobject = [test_class new "Ron"]
print "New object created"
$myobject hello
print "Done"
