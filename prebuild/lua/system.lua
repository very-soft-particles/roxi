require "io"
require "string"

local System = {}


System["counter"] = 0

function System.new(name, input_components, output_components)

  local system = {}
  system["name"] = name
  system["id"] = System.counter
  system["input_components"] = input_components
  system["output_components"] = output_components

  System.counter = System.counter + 1

  return system
end

return System
