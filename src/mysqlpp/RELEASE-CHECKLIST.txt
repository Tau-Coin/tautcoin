- Change version number in configure.ac and mysql++.bkl.

  All other places the version number occurs are generated files
  created from one of these two.  If there's a corresponding *.in file
  for the one you're looking at, the version number was substituted in
  by autoconf from configure.ac.  Otherwise, the file was most likely
  created by the build system using the version number in mysql++.bkl.

- Run "make abicheck".  There should be no changes.

  You may have to run the following command in the current "stable"
  directory before this will succeed, since it depends on there being
  an ACC dump file in place already.

      $ abi-compliance-checker -lib mysqlpp -dump abi.xml

  ("Stable" is assumed to be in ../3.1.0 relative to the svn "head"
  checkout, as I write this.)

  This dependence on an existing ABI dump file is deemed reasonable
  since the ABI of the stable version had better not be changing!
  Plus, it saves some processing time, since ACC can load the stable
  ABI info without re-parsing its headers and library file.

- Re-bootstrap the system in pedantic mode, then do a clean rebuild.
  Fix any new errors and warnings.

  Known bogus warnings:

  - Query's std::basic_ios<> base class is not being initialized.
    Yes, we know.  We don't care.

  - The "==" float comparisons in lib/stadapter.cpp are harmless.
    They're comparisons against special NaN and infinity constants.
    Those are safe.

- Re-bootstrap it again without "pedantic", to avoid shipping the
  pedantic build files.
