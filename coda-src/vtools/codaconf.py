import os, sys

default_codaconfpath = "@sysconfdirx@:/usr/local/etc/coda:/etc/coda:"
conffile = None
codaconf_table = {}

def codaconf_lookup(name, defaultvalue):
    return codaconf_find(name, defaultvalue, False)

def codaconf_find(name, value, replace):
    global codaconf_table
    if name in codaconf_table:
        if replace and value:
            codaconf_table[name] = value
        return codaconf_table[name]
    if value == None:
        return None
#ifdef CONFWRITE
    if not replace:
        try:
            conf = fopen(conffile, "a")
            conf.write(name + "=" + "\"" + value + "\"\n")
            conf.close()
        except:
            sys.stderr.write("Cannot write configuration file %s\n"
                  % conffile)
            return None
#endif
    codaconf_table[name] = value
    return value

def codaconf_init_one(cf):
    global conffile
    try:
        conf = open(cf, "r")
    except:
        sys.stderr.write("Cannot read configuration file %s, will use\
                         default values.\n" % cf)
        return -1
    if cf != conffile:
        conffile = cf

    lines = conf.readlines()
    lineno = 0
    for line in lines:
        lineno += 1
        line.strip()
        if line == "\n" or line == "" or "#" in line:
            continue
        pairs = line.split("=")
        name = pairs[0].strip()
        value = pairs[1].strip()
        if value[0] == '"' or value[0] == '\'':
            value[0] = ""
        if value[-1] == '"' or value[-1] == '\'':
            value[-1] = ""
        found_value = codaconf_find(name, value, True)
#ifdef confdebug
        sys.stdout.write("line: %d, name: %s, value: %s"
                         % (lineno, name, value))
        if found_value:
            sys.stdout.write("stored-value: '%s'\n" % found_value)
        else:
            sys.stdout.write("not found?\n")
        sys.stdout.flush()
#endif
    conf.close()
    return 0

def codaconf_init(confname):
    codaconfpath = os.getenv("CODACONFPATH", default_codaconfpath)
    paths = codaconfpath.split(":")
    conffile = None
    for path in paths:
        for filename in os.listdir(paths):
            abspath = os.path.abspath(filename)
            if confname in filename and os.access(abspath, os.R_OK):
                conffile = abspath
                break
        if conffile:
            break
    if conffile is None or codaconf_init_one(conffile) != 0:
        return -1
    return 0

