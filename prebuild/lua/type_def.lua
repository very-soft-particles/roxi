local Type = {}

Type["PRIMITIVE"] = 0
Type["ENUM"] = 1
Type["STRUCT"] = 2

function Type.new(name, kind, members_or_alias_name)
  type = {}

  type["name"] = name
  type["kind"] = kind
  type["members"] = members_or_alias_name

  return type
end

function Type.write(types, out_file_path)
  io.output(out_file_path)
  for type in types do
    if type.kind == 0 then
      io.write("PRIMITIVE,", type.name, ",", type.members, ",")
    elseif type.kind == 1 then
      io.write("ENUM,", type.name, ",")
      for member in type.members do
        io.write(member.name, ",")
      end
    elseif type.kind == 2 then
      io.write("STRUCT,", type.name, ",")
      for member in type.members do
        io.write(member.type, ",", member.name, ",")
      end
    end
    io.write("\n")
  end
  io.close(out_file_path)
end

return Type
