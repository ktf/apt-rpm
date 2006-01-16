#!/usr/bin/python
import sys, os
import rpm

def main():
    if len(sys.argv) == 1:
        sys.exit("Usage: convert.py <hdlist file|rpms dir> ...")

    if not hasattr(rpm, "headerFromPackage"):
        ts = rpm.TransactionSet()
    else:
        ts = None

    for entry in sys.argv[1:]:
        if os.path.isfile(entry):
            for header in rpm.readHeaderListFromFile(entry):
                name = header[rpm.RPMTAG_NAME]
                for filename in header[rpm.RPMTAG_FILENAMES]:
                    print filename, name
        if os.path.isdir(entry):
            for filename in os.listdir(entry):
                if filename.endswith(".rpm"):
                    filepath = os.path.join(entry, filename)
                    file = open(filepath)
                    if ts:
                        header = ts.hdrFromFdno(file.fileno())
                    else:
                        header = rpm.headerFromPackage(file.fileno())[0]
                    name = header[rpm.RPMTAG_NAME]
                    for filename in header[rpm.RPMTAG_FILENAMES]:
                        print filename, name
                    file.close()

if __name__ == "__main__":
    main()

