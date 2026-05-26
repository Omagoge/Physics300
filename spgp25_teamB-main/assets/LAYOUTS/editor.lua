local layout = require("layout")
layout.name = "editor"

layout.add("move_camera", 2)
layout.bind("move_camera", { up = "w", down = "s", left = "a", right = "d" })

layout.add("zoom_camera", 1)
layout.bind("zoom_camera", { left = "q", right = "e" })

layout.add("rotate_camera", 0)
layout.bind("rotate_camera", "mouse/r")

layout.add("mouse", 2)
layout.bind("mouse", "mouse/position")

layout.add("mouse_click", 0)
layout.bind("mouse_click", "mouse/l")

return layout