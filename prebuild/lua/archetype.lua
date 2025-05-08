
local Archetype = {}

Archetype["counter"] = 0

function Archetype.new(name, components, max_entities)
  local archetype = {}

  archetype["id"] = Archetype.counter
  archetype["name"] = name
  archetype["components"] = components
  archetype["size"] = max_entities

  Archetype.counter = Archetype.counter + 1

  return archetype
end

function Archetype.write(archetypes, out_file_path)
  io.output(out_file_path)
  for archetype in archetypes do
    io.write(archetype.name, ",", archetype.size, ",")
    for component in archetype.components do
      io.write(component.name, ",")
    end
    io.write("\n")
  end
  io.close(out_file_path)
end

return Archetype
