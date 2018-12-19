# Test the standard library.

$library.path append "../libs/.libs"
import stdlib

#=============================================================================#
#
try {
  raise "I threw an error"
  return 1
} catch err {
  if {$err == "I threw an error"} then {
    print "I caught an error ->" $err
  } else {
    print "I caught an unexpected error ->" $err
    global result = 1
  }
}

#=============================================================================#
#
global myvar = "whatever"
print "\$myvar = $myvar"

global myvar =
try {
  print "\$myvar = $myvar"
  return 1
}
