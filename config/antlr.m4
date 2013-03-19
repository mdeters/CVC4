##
# Check for ANTLR's antlr3 script.
# Will set ANTLR to the location of the script.
##
AC_DEFUN([AC_PROG_ANTLR], [
  AC_ARG_VAR([ANTLR],[location of the antlr3 script])

  # Check the existence of the runantlr script
  if test "x$ANTLR" = "x"; then
    AC_PATH_PROG(ANTLR, [antlr3])
  else
    AC_MSG_CHECKING([antlr3 script ($ANTLR)])
    if test ! -e "$ANTLR"; then
      AC_MSG_RESULT([not found])
      unset ANTLR
    elif test ! -x "$ANTLR"; then
      AC_MSG_RESULT([not executable])
      unset ANTLR
    else
      AC_MSG_RESULT([OK])
    fi
  fi
  if test "x$ANTLR" = "x"; then
    AC_MSG_WARN(
[No usable antlr3 script found. Make sure that the parser code has
been generated already. To obtain ANTLR see <http://www.antlr3.org/>.]
    )
    ANTLR_VERSION=
  else
    ANTLR_VERSION="`$ANTLR -version 2>&1 | sed 's,.*Version  *\([[0-9.]]*\).*,\1,'`"
    case "$ANTLR_VERSION" in
      3.2|3.2.*) ANTLR_VERSION=3.2 ;;
      3.4|3.4.*) ANTLR_VERSION=3.4 ;;
      *) AC_MSG_WARN([unknown version of antlr: $ANTLR_VERSION]);;
    esac
  fi
])

##
# Check the existence of the ANTLR3 C++ runtime library and headers
# Will set ANTLR_INCLUDES to the location of the ANTLR C++ headers
##
AC_DEFUN([AC_LIB_ANTLR],[
  AC_ARG_VAR(ANTLR_HOME, [path to libantlr3c installation])

  # Get the location of the ANTLR3 C++ includes
  AC_ARG_WITH(
    [antlr-dir],
    AS_HELP_STRING(
      [--with-antlr-dir=PATH],
      [path to ANTLR C++ headers]
    ),
    ANTLR_PREFIXES="$withval",
    ANTLR_PREFIXES="$ANTLR_HOME /usr/local /usr /opt/local /opt"
  )

  AC_MSG_CHECKING(for ANTLR3 C++ runtime library)

  # Use C++ and remember the variables we are changing
  AC_LANG_PUSH(C++)
  OLD_CPPFLAGS="$CPPFLAGS"

  # Try all the includes/libs set in ANTLR_PREFIXES
  for antlr_prefix in $ANTLR_PREFIXES
  do
    antlr3includes=
    if test -e "$antlr_prefix/antlr3.hpp"; then
      antlr3includes="-I$antlr_prefix"
    fi
    if test -e "$antlr_prefix/include"; then
      antlr3includes="$includes -I$antlr_prefix/include"
    fi
    CPPFLAGS="$OLD_CPPFLAGS $antlr3includes"
    AC_LINK_IFELSE([AC_LANG_SOURCE(
      [
        #include <antlr3.hpp>

        int main() {
          antlr3::FileUtils<void>::AntlrFopen;
          return 0;
        }
      ])],
      [
        AC_MSG_RESULT(found in $antlr_prefix)
        ANTLR_INCLUDES="$antlr3includes"
        ANTLR_LDFLAGS=
        break
      ],
          [
            AC_MSG_RESULT(no)
            AC_MSG_ERROR([ANTLR3 C++ runtime not found, see <http://www.antlr3.org/>])
          ]
    )
  done

  # Return the old compile variables and pop the language.
  CPPFLAGS="$OLD_CPPFLAGS"
  AC_LANG_POP()

  # Define the ANTLR include/libs variables
  AC_SUBST(ANTLR_INCLUDES)
  AC_SUBST(ANTLR_LDFLAGS)
])
