# Test the standard library.

$library.path append "../libs/.libs"
import stdlib

global result = 0

try {
  raise "I threw an error"
  global result = 1
} catch err {
  if {$err == "I threw an error"} then {
    print "I caught an error ->" $err
  } else {
    print "I caught an unexpected error ->" $err
    global result = 1
  }
}

return $result
