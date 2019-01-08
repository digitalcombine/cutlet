# Test the standard library.

$library.path append "../libs/.libs"
import stdlib

#=============================================================================#
# Testing try catch throw

try {
  raise "I threw an error"
  return 1
} catch err {
  if {$err == "2:3: I threw an error"} then {
    print "I caught an error ->" $err
  } else {
    print "I caught an unexpected error ->" $err
    return 1
  }
}

#=============================================================================#
# Testing removal of global variable

global myvar = "whatever"
print "\$myvar = $myvar"

global myvar =
try {
  print "\$myvar = $myvar"
  return 1
}

#=============================================================================#
# Testing while loops

global result = ""
while {$result <> "+++++"} do {
  global result = "${result}+"
}

if {$result <> "+++++"} then {
  print "\$result <> \"+++++\" ->  \"${result}\""
  return 1
}

global result = ""
while {$result <> "+++++"} {
  global result = "${result}+"
  if {$result == "++++"} {
    break
  }
}

if {$result <> "++++"} then {
  print "\$result <> \"++++\" ->  \"${result}\""
  return 1
}

global result = ""
while {$result < "+-+-+-++"} {
  global result = "${result}+"
  if {$result == "+-+-+-+"} {
    continue
  }
  global result = "${result}-"
}

if {$result <> "+-+-+-++-"} then {
  print "\$result <> \"+-+-+-++-\" ->  \"${result}\""
  return 1
}