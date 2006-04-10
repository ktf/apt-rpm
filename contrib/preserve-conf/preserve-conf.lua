if confget("RPM::Preserve-Config/b", "false") == "true" then
	num = 0
	pkgs = pkglist()
	for i, pkg in ipairs(pkgs) do
		if statinstall(pkg) then
			inp = io.popen("LANG=C /bin/rpm -V --nodeps --nodigest --noscripts --nosignature "..pkgname(pkg).." 2> /dev/null")
			for line in inp.lines(inp) do
				if string.byte(line, 10) == string.byte("c") then
					num = num + 1
					markkeep(pkg)
				end
			end
			io.close(inp)
		end
	end
	if num > 0 then
		print("\nHolding back "..num.." packages because of changed configuration")
	end
end
-- vim:ts=4
