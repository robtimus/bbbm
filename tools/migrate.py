#!/usr/bin/env python

import sys, os, getopt, re, xml.dom.minidom

# parse command line options

try:
	options, files = getopt.getopt(sys.argv[1:], "hi:o:", ["help", "input=", "output="])
except getopt.error, e:
	sys.stderr.write("%s: %s\n" % (sys.argv[0], str(e)))
	sys.stderr.write("Try %s --help for more information\n" % sys.argv[0])
	sys.exit(1)

input_file = "~/.bbbm/bbbm.cfg"
output_file = "~/.bbbm/bbbm.xml"

for option, value in options:
	if option == "-h" or option == "--help":
		print "Usage: %s [OPTIONS]" % sys.argv[0]
		print "Converts an old style BBBM configuration file into a new style one"
		print
		print "  -h, --help            Output this help and exit"
		print "  -i, --input=<input>   Convert <input>; defaults to ~/.bbbm/bbbm.cfg"
		print "  -o, --output=<output> Convert to <output>; defaults to ~/bbbm/bbbm.xml"
		sys.exit(0)
	elif option == "-i" or option == "--input":
		input_file = value
	elif option == "-o" or option == "--output":
		output_file = value

print "Converting '%s' to '%s'\n" % (input_file, output_file)

set_command = None
view_command = None
thumb_width = None
thumb_height = None
thumb_column_count = None
filename_as_label = None
filename_as_title = None
commands = []
labels = []

# parse data

def ensure_size(l, size, filler = None):
	if len(l) < size:
		l += [filler] * (size - len(l))
	return l

def report_invalid_value(line, value):
	global input_file
	sys.stderr.write("%s: illegal value on line %d of '%s': %s\n" % (sys.argv[0], line + 1, input_file, value))
	sys.exit(1)

try:
	fin = open(os.path.expanduser(input_file), "r")
	lines = fin.readlines()
	fin.close()
except IOError, e:
	sys.stderr.write("%s: could not read '%s': %s\n" % (sys.argv[0], input_file, str(e)))
	sys.exit(1)

for i in xrange(len(lines)):
	line = lines[i][:-1].lstrip()
	if not line or line[0] == "#":
		continue;
	try:
		option, value = line.split("=", 1)
		option = option.strip()
		value = value.strip()
	except ValueError:
		sys.stderr.write("%s: error on line %d of '%s': %s\n" % (sys.argv[0], i + 1, input_file, line))
		sys.exit(1)

	if option == "set_command":
		set_command = value
	elif option == "view_command":
		view_command = value
	elif option == "thumb_size":
		match = re.match(r"^(\d+)x(\d+)$", value)
		if match != None:
			thumb_width = int(match.group(1))
			thumb_height = int(match.group(2))
		else:
			report_invalid_value(i, value)
	elif option == "thumb_cols":
		try:
			thumb_column_count = int(value)
		except ValueError:
			report_invalid_value(i, value)
	elif option == "filename_label":
		if "true" == value.lower():
			filename_as_label = True
		elif "false" == value.lower():
			filename_as_label = False
		else:
			report_invalid_value(i, value)
	elif option == "filename_title":
		if "true" == value.lower():
			filename_as_title = True
		elif "false" == value.lower():
			filename_as_title = False
		else:
			report_invalid_value(i, value)
	elif option.startswith("command"):
		try:
			index = int(option[7:])
			commands = ensure_size(commands, index + 1)
			commands[index] = value
		except ValueError:
			pass
	elif option.startswith("cmd_label"):
		try:
			index = int(option[9:])
			labels = ensure_size(labels, index + 1)
			labels[index] = value
		except ValueError:
			pass

commands = ensure_size(commands, len(labels))
labels = ensure_size(labels, len(commands))

# report findings

print "Found the following values:"
if set_command != None:
	print "- set command: %s" % set_command
if view_command != None:
	print "- view command: %s" % view_command
if thumb_width != None and thumb_height != None:
	print "- thumb size: %dx%d" % (thumb_width, thumb_height)
if thumb_column_count != None:
	print "- thumb column count: %d" % thumb_column_count
if filename_as_label != None:
	print "- filename as label: %s" % str(filename_as_label).upper()
if filename_as_title != None:
	print "- filename as title: %s" % str(filename_as_title).upper()
print "- commands:"
for i in xrange(len(commands)):
	print "  - '%s' (%s)" % (commands[i], ("no label", labels[i])[labels[i] != None])
print

# check for overwrite

if os.path.isfile(os.path.expanduser(output_file)):
	valid_options = {"yes": True, "ye": True, "y": True, "no": False, "n": False}
	overwrite_file = False
	while True:
		sys.stderr.write("File '%s' already exists. Overwrite? [Y/n] " % output_file)
		choice = raw_input().lower()
		if choice == '':
			overwrite_file = True
		elif choice in valid_options:
			overwrite_file = valid_options[choice]
			break

	if not overwrite_file:
		sys.stderr.write("Not overwriting file '%s'. Aborting\n" % output_file)
		sys.exit(1)

# output

commands = ensure_size(commands, 10)
labels = ensure_size(labels, 10)

def escape_xml(value):
	text = xml.dom.minidom.Text();
	text.data = value
	return text.toxml()

try:
	fout = open(os.path.expanduser(output_file), "w")
except IOError, e:
	sys.stderr.write("%s: could not write to '%s': %s\n" % (sys.argv[0], output_file, str(e)))
	sys.exit(1)

fout.write("<!--\n")
fout.write("  bbbm configuration (version 0.8)\n")
fout.write("-->\n")
fout.write("<?xml version=\"1.0\"?>\n")
fout.write("<bbbm>\n")
if (thumb_width != None and thumb_height != None) or thumb_column_count != None:
	fout.write("  <thumbs>\n")
	if thumb_width != None and thumb_height != None:
		fout.write("    <size width=\"%d\" height=\"%d\" />\n" % (thumb_width, thumb_height))
	if thumb_column_count != None:
		fout.write("    <column-count>%d</column-count>\n" % thumb_column_count)
	fout.write("  </thumbs>\n")
if filename_as_label != None or filename_as_title != None:
	fout.write("  <menu>\n")
	if filename_as_label != None:
		fout.write("    <filename-as-label>%s</filename-as-label>\n" % str(filename_as_label).lower())
	if filename_as_title != None:
		fout.write("    <filename-as-title>%s</filename-as-title>\n" % str(filename_as_title).lower())
	fout.write("  </menu>\n")
fout.write("  <commands>\n")
if set_command != None:
	fout.write("    <set-command>%s</set-command>\n" % escape_xml(set_command))
else:
	fout.write("    <set-command />\n")
if view_command != None:
	fout.write("    <command>\n")
	fout.write("      <command>%s</command>\n" % escape_xml(view_command))
	fout.write("      <label>View</label>\n")
	fout.write("    </command>\n")
for i in xrange(len(commands)):
	if commands[i] != None or labels[i] != None:
		fout.write("    <command>\n")
		if commands[i] != None:
			fout.write("      <command>%s</command>\n" % escape_xml(commands[i]))
		if labels[i] != None:
			fout.write("      <label>%s</label>\n" % escape_xml(labels[i]))
		fout.write("    </command>\n")
	else:
		fout.write("    <command />\n")
fout.write("  </commands>\n")
fout.write("</bbbm>\n")

fout.close() 

print "Wrote converted configuration to '%s'" % output_file
