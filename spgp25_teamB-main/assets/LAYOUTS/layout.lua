local states
local actions
local name
local format = "input_layout"

name = {}
states = {}
actions = {}

--- Creates a new Action
--- @param name string Name of the Action
--- @param type integer 0: Button(bool), 1: 1D axis(float), 2: 2D axis(x,y)
--- @param state? any States (or number of states) the action can happen (use '-' for exclusion)
local add_fn = function(name, dimension, state)
	actions[name] = { type = dimension }
	if type(state) == string then
		actions[name].states = {}
		table.insert(actions[name].states, state)
	elseif type(state) == table then
		actions[name].states = state
	end
end

--- Creates a new State
local add_state_fn = function(name)
	table.insert(states, name)
end

--- Binds an input to an Action
--- @param name string Name of the Action
--- @param bind any String with the key or Table {up, down, left, right}, {x, y}, {left, right}
--- @param modifiers? table Table with modifiers for the bind
local bind_fn = function(name, bind, modifiers)
	local action = actions[name]

	if action then
		if action.binds == nil then
			action.binds = { }
		end

		if modifiers then
			if type(bind) == "table" then
				bind.modifiers = modifiers
				table.insert(action.binds, bind)
			else
				local new_bind = {
					key = bind,
					modifiers = modifiers or {}
				}
				table.insert(action.binds, new_bind)
			end
		else
			table.insert(action.binds, bind)
		end
	else
		local info = debug.getinfo(1, "Sl") 
		local file_line = "[" .. info.source .. ":" .. info.currentline .. "]"
		print(file_line .. "Input System Error: Action '" .. name .. "' not found.")
	end
end

return {
	format = format,
	name = name,
	states = states,
	actions = actions,
	create = create_fn,
	add = add_fn,
	add_state = add_state_fn,
	bind = bind_fn
}
