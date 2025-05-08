
local Tag = {}

Tag["counter"] = 0

function Tag.new(name)
  local tag = {}

  tag["id"] = Tag.counter
  tag["name"] = name

  Tag.counter = Tag.counter + 1

  return tag
end

local Component = {}

Component["counter"] = 0

local Member = {}

function Member.new(type, name)
  local member = {}

  member["type"] = type
  member["name"] = name

  return member
end

function Component.new(name, members)
  local component = {}

  component["id"] = Component.counter;
  component["name"] = name
  component["members"] = members

  Component.counter = Component.counter + 1

  return component
end

function Component.write(components, out_file_path)
  io.output(out_file_path)
  for component in components do
    io.write(component.name, ",")
    for member in component.members do
      io.write(member.type, ",", member.name, ",")
    end
    io.write("\n")
  end
  io.close(out_file_path)
end

function Tag.write(tags, out_file_path)
  io.output(out_file_path)
  for tag in tags do
    io.write(tag.name, ",\n")
  end
  io.close(out_file_path)
end

return Tag, Component, Member
