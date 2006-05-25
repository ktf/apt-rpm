-- This script will handle Allow-Duplicated packages when more
-- than one is available during an install operation, and will
-- also upgrade these packages in a dist-upgrade operation (if
-- they match a regex in RPM::Allow-Duplicated-Upgrade).
--
-- This script must be plugged in the following slots:
--
--   Scripts::AptGet::Install::SelectPackage
--   Scripts::AptGet::DistUpgrade
--   Scripts::Synaptic::DistUpgrade
--
-- Author: Gustavo Niemeyer <niemeyer@conectiva.com>

function realname(name)
    local s, e, name = string.find(name, "(.+)#")
    return name
end

if script_slot == "Scripts::AptGet::Install::SelectPackage" then
    -- Automatically select the newest package if multiple
    -- Allow-Duplicated packages are available.
    local goodpkg = packages[1]
    local goodpkgname = realname(pkgname(goodpkg))
    if goodpkgname then
        -- Check if every package has the same real name, and
        -- leave only the one with the greatest version, if
        -- that's the case.
        for i = 2, table.getn(packages) do
            local nextpkg = packages[i]
            local nextpkgname = realname(pkgname(nextpkg))
            if nextpkgname ~= goodpkgname then
                goodpkg = nil
                break
            end
            if not pkgvercand(goodpkg)
               or pkgvercand(nextpkg) and
                  verstrcmp(verstr(pkgvercand(goodpkg)),
                    verstr(pkgvercand(nextpkg))) == -1 then
                goodpkg = nextpkg
            end
        end
        if goodpkg and pkgvercand(goodpkg) then
            selected = goodpkg
        end
    end
    if not selected then
        -- Strip #... from package names if we can't find a good solution.
        for i, name in ipairs(packagenames) do
            local name = realname(name)
            if name and name ~= virtualname then
                packagenames[i] = name
            end
        end
    end
end

if script_slot == "Scripts::AptGet::DistUpgrade" or
   script_slot == "Scripts::AptGet::Upgrade" or
   script_slot == "Scripts::Synaptic::DistUpgrade" or
   script_slot == "Scripts::Synaptic::Upgrade" then
    -- Automatically install newer versions of all packages which
    -- are registered in the Allow-Duplicated scheme and are matched
    -- by the regular expressions in RPM::Allow-Duplicated-Upgrade.

    -- Compile expressions with package names which should
    -- be considered for upgrade.
    local updatelist = confgetlist("RPM::Allow-Duplicated-Upgrade")
    for i, expr in ipairs(updatelist) do
        updatelist[i] = rex.new(expr)
    end

    if table.getn(updatelist) ~= 0 then

        -- Gather information about Allow-Duplicated packges.
        local canddups = {}
        local curdups = {}
        for i, pkg in pairs(pkglist()) do 
            local name = realname(pkgname(pkg))
            if name then
                if pkgvercur(pkg) then
                    if not curdups[name] then
                        curdups[name] = {}
                    end
                    table.insert(curdups[name],
                             verstr(pkgvercur(pkg)))
                elseif pkgvercand(pkg) then
                    if not canddups[name] then
                        canddups[name] = {}
                    end
                    table.insert(canddups[name],
                             verstr(pkgvercand(pkg)))
                end
            end
        end

        -- Compile expressions with package names which should be hold.
        local holdlist = confgetlist("RPM::Hold")
        for i, expr in ipairs(holdlist) do
            holdlist[i] = rex.new(expr)
        end

        -- Remove packages without any matches in updatelist, or with
        -- any matches in holdlist.
        for name, _ in pairs(curdups) do
            local found = false
            for i, expr in ipairs(updatelist) do
                if expr:match(name) then
                    found = true
                    break
                end
            end
            if found then
                for i, expr in ipairs(holdlist) do
                    if expr:match(name) then
                        found = false
                        break
                    end
                end
            end
            if not found then
                curdups[name] = nil
            end
        end

        -- Mark the newest packages for installation.
        for name, _ in pairs(curdups) do
            if canddups[name] then
                -- Check the best candidate version.
                local bestver = nil
                for i, ver in ipairs(canddups[name]) do
                    if not bestver or
                       verstrcmp(ver, bestver) == -1 then
                        bestver = ver
                    end
                end

                -- Now check if it's newer than all installed
                -- versions.
                for i, ver in ipairs(curdups[name]) do
                    if verstrcmp(ver, bestver) == 1 then
                        bestver = nil
                        break
                    end
                end

                -- Finally, mark it for installation.
                if bestver then
                    markinstall(name.."#"..bestver)
                end
            end
        end
    end
end

-- vim:ts=4:sw=4:et
