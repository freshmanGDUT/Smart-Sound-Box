/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/ doc++ / doc++ /
  tb
  s/ $/ doc++ /
  :b
  s/^/# Packages using this file:/
}
