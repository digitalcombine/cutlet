# Testing Sandboxes

$library.path append "../libs/.libs"
import stdlib

global result = 0

#=============================================================================#
# Test 1: Attempt to call a core function in an empty sandbox.

global sandbox1 [sandbox]
try {
  $sandbox1 eval {
    print "Is there anybody out there?"
  }

  # We shouldn't get here.
  global result = 1

} catch err {
  print "Successfully caught error: $err"
}

#=============================================================================#
# Test 2: We link the print function into the new sandbox and attempt to call
#         it.
$sandbox1 link print
try {
  $sandbox1 eval {
    print "Is there anybody out there?"
  }
  print "Yes, we can hear you."

} catch err {
  print $err
  return 1
}

#=============================================================================#
# Test 3: Attempt to access ourselves to bypass the sandbox
$sandbox1 link print
try {
  $sandbox1 eval {
    $sandbox1 link print
  }
  global result = 1
} catch err {
  print "Successfully caught error: $err"
}

#=============================================================================#
# Test 4: Importing libraries.
$sandbox1 global library.path = $library.path
$sandbox1 link import

try {
  $sandbox1 eval {
    import stdlib
  }
} catch err {
  global result = 1
}

if {$result == 0} then {
  print "All of our sanbox tests passed!"
}
return $result
