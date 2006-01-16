
function installdevel(name)
	pkg = pkgfind(name.."-devel")
	if pkg and not pkgvercur(pkg) then
		markinstall(pkg)
	end
	pkg = pkgfind(name.."-devel-static")
	if pkg and not pkgvercur(pkg) then
		markinstall(pkg)
	end
end

for i, pkg in pairs(pkglist()) do
	if pkgvercur(pkg) then
		installdevel(pkgname(pkg))
		name = pkgname(pkg)
		while string.gfind(string.sub(name, -1), "%d")() do
			name = string.sub(name, 1, -2)
		end
		if name ~= pkgname(pkg) then
			installdevel(name)
		end
	end
end
