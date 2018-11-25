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

  method goodbye {} {} {
    print "See ya later $the_name"
  }
}
