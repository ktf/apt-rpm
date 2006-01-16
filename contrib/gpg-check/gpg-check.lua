if confget("RPM::GPG-Check/b", "true") == "false" then
    return
end

if table.getn(files_install) < 1 then
    return
end

hash = '###########################################'
hashestotal = string.len(hash)
interactive = confget("RPM::Interactive/b", "true")
quiet = tonumber(confget("quiet", 0))

function printhash(amount, total)
    percent = amount/total*100
    if interactive == "true" then
	nrhash = hashestotal - hashestotal / total * amount
	line = string.format("%-44s[%3d%%]", string.sub(hash, nrhash), percent)
	io.stdout.write(io.stdout, line)
	io.stdout.flush(io.stdout)
	for i = 1, string.len(line) do
	    io.stdout.write(io.stdout, '\b')
	end
    else
	io.stdout.write(io.stdout, string.format("%%%% %f\n", percent))
    end
end
	
function showerrors(i, msg)
    apterror(msg)
end

good = 1
unknown = 0
illegal = 0
unsigned = 0
errors = {}

skiplist = confgetlist("RPM::GPG::Skip-Check", "")

io.stdout.write(io.stdout, string.format("%-28s", _("Checking GPG signatures...")))
if interactive == "false" then
	io.stdout.write(io.stdout, '\n')
end
for i, file in ipairs(files_install) do
    skipthis = false
    for j, skip in ipairs(skiplist) do
	start = string.find(pkgname(pkgs_install[i]), skip)
	if start then
	    skipthis = true
	    aptwarning(_("Skipped GPG check on "..pkgname(pkgs_install[i])))
	    break
	end
    end
    if quiet == 0 then
	printhash(i, table.getn(files_install))
    end
    if skipthis == false then
	inp = io.popen("LANG=C /bin/rpm --checksig  "..file.." 2>&1")
 
	for line in inp.lines(inp) do
	    if string.find(line, "gpg") then
		break
	    elseif string.find(line, "GPG") then
		table.insert(errors, _("Unknown signature "..line))
		unknown = unknown + 1
		good = nil
	    elseif string.find(line, "rpmReadSignature") then
		table.insert(errors, _("Illegal signature "..line))
		illegal = illegal + 1
		good = nil
	    else
		table.insert(errors, _("Unsigned "..line))
		unsigned = unsigned + 1
		good = nil
	    end
	end
	io.close(inp)
    end
end
if interactive == "true" then
    io.stdout.write(io.stdout, '\n')
end

if not good then
    table.foreach(errors, showerrors)
    apterror(_("Error(s) while checking package signatures:\n"..unsigned.." unsigned package(s)\n"..unknown.." package(s) with unknown signatures\n"..illegal.." package(s) with illegal/corrupted signatures"))
end

-- vim::sts=4:sw=4
