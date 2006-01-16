#!/usr/bin/python

# apt-groupinstall v0.2
# groupinstall helper backend for for RHL/RHEL/FC systems 
# by pmatilai@welho.com

import rhpl.comps, sys

def findgroup(comps, grpname):
	if comps.groups.has_key(grpname):
		return comps.groups[grpname]
	for group in comps.groups.values():
		if group.id == grpname:
			return group

def grouppkgs(comps, grpname, recursive = 0, showall = 0):
	group = findgroup(comps, grpname)
	pkgs = []
	if group and recursive:
		for grp in group.groups:
			pkgs += grouppkgs(comps, grp, recursive, showall)
	if group and group.packages:
		for pkg in group.packages:
			type, name = group.packages[pkg]
			if not showall and type == "optional":
				continue
			pkgs.append(pkg)
	return pkgs

def groupnames(comps, showhidden = 0):
	if not synaptic:
		print "%-40s %s" % ("Group name", "Description")
		print "%-40s %s" % ("----------", "-----------")
	for group in comps.groups.values():
		if group.packages:
			if not showhidden and not group.user_visible:
				continue
			if synaptic:
				print "u %s\t%s" % (group.id, group.name)
			else:
				print "%-40s %s" % (group.id, group.name)

def showgroup(comps, grpname, showall = 0):
	group = findgroup(comps, grpname)
	if not group or not group.packages:
		print "No such group: %s" % grpname
		return
	if not synaptic: 
		print "Group:\n    %s" % group.id
	print "Description:\n    %s" % group.description
	print "Required groups: "
	for grp in group.groups:
		print "    %s" % grp
	print "Packages: "
	for pkg in grouppkgs(comps, grpname, recursive=0, showall=showall):
		print "    %s" % pkg
		
def usage():
	print "Usage:\n %s [-t] [-p <path>] [-h] --list-tasks" % sys.argv[0]
	print " %s [-t] [-p <path>] [-a] --task-desc <group> [--task-desc <group2>...]" % sys.argv[0]
	print " %s [-t] [-p <path>] [-a] [-r] --task-packages <group> [--task-packages <group>...]" % sys.argv[0]
	sys.exit(1)

if __name__ == "__main__":
	import getopt

	recursive = 0
	showhidden = 0
	synaptic = 1
	showall = 0
	comps = None
	groups = []
	cmd = None
	compspath = "/usr/share/comps/i386/comps.xml"
	
	try:
		optlist, args = getopt.getopt(sys.argv[1:], 'arhp:t',
						['task-desc=', 'list-tasks', 'task-packages='])
	except getopt.error:
		usage()


	for opt, arg in optlist:
		if opt == '--task-desc':
			cmd = "showgroup"
			groups.append(arg)
		elif opt == '--task-packages':
			groups.append(arg)
			cmd = "grouppkgs"
		elif opt == '--list-tasks':
			cmd = "groupnames"
		elif opt == '-r':
			recursive = 1
		elif opt == '-h':
			showhidden = 1
		elif opt == '-a':
			showall = 1
		elif opt == '-p':
			compspath = arg
		elif opt == '-t':
			synaptic = 0
		else:
			usage()

	if not cmd: usage()

	try:
		comps = rhpl.comps.Comps(compspath)
	except:
		print "Unable to open %s!" % compspath
		sys.exit(1)

	if cmd == "groupnames":
		groupnames(comps, showhidden)
	elif cmd == "grouppkgs":
		for grp in groups:
			for pkg in grouppkgs(comps, grp, recursive, showall):
				print "%s" % pkg
	elif cmd == "showgroup":
		for grp in groups:
			showgroup(comps, grp, showall)
	else:
		usage()
	
# vim:ts=4:sts=4
