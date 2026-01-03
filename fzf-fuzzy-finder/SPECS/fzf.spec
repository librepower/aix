Name:           fzf
Version:        0.46.1
Release:        1.aix7.3.sixe
Summary:        A command-line fuzzy finder
License:        MIT
Group:          Applications/Text
URL:            https://github.com/junegunn/fzf
Vendor:         SIXE
Packager:       Hugo Blanco <hugo.blanco@sixe.eu>

%description
fzf is a general-purpose command-line fuzzy finder.

It's an interactive Unix filter for command-line that can be used with 
any list; files, command history, processes, hostnames, bookmarks, 
git commits, etc.

Features:
- Portable (single binary, no dependencies)
- Incredibly fast (500,000 items in < 1 second)
- Full-featured with preview window
- Keyboard shortcuts for bash/zsh
- Works with any list

Compiled for AIX by SIXE - IBM Business Partner
https://sixe.eu | Part of LibrePower initiative (https://librepower.org)

%install
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/share/fzf/shell
mkdir -p %{buildroot}/opt/freeware/share/man/man1

# Main binary
cp /tmp/fzf/fzf %{buildroot}/opt/freeware/bin/

# Shell integration scripts (from upstream)
cp /tmp/fzf/shell/completion.bash %{buildroot}/opt/freeware/share/fzf/shell/
cp /tmp/fzf/shell/completion.zsh %{buildroot}/opt/freeware/share/fzf/shell/
cp /tmp/fzf/shell/key-bindings.bash %{buildroot}/opt/freeware/share/fzf/shell/
cp /tmp/fzf/shell/key-bindings.zsh %{buildroot}/opt/freeware/share/fzf/shell/

# Man page
cp /tmp/fzf/man/man1/fzf.1 %{buildroot}/opt/freeware/share/man/man1/

# LibrePower helper scripts
cat > %{buildroot}/opt/freeware/bin/fzf-rpm << 'SCRIPT'
#!/opt/freeware/bin/bash
# fzf-rpm: Interactive RPM browser for AIX
# Part of LibrePower (https://librepower.org)
#
# Usage: fzf-rpm [filter]
#   Without arguments: interactive browser
#   With argument: filter and show matching packages

export PATH=/opt/freeware/bin:$PATH

if [ -n "$1" ]; then
    rpm -qa | sort | fzf --filter="$1"
else
    selected=$(rpm -qa | sort | fzf \
        --prompt="RPM> " \
        --header="Select package (Enter=info, Ctrl-C=exit)" \
        --preview="rpm -qi {}" \
        --preview-window=right:50%)
    
    if [ -n "$selected" ]; then
        echo ""
        rpm -qi "$selected"
    fi
fi
SCRIPT

cat > %{buildroot}/opt/freeware/bin/fzf-proc << 'SCRIPT'
#!/opt/freeware/bin/bash
# fzf-proc: Interactive process viewer for AIX
# Part of LibrePower (https://librepower.org)
#
# Usage: fzf-proc [filter]
#   Without arguments: interactive browser
#   With argument: filter and show matching processes

export PATH=/opt/freeware/bin:$PATH

if [ -n "$1" ]; then
    ps -ef | fzf --header-lines=1 --filter="$1"
else
    ps -ef | fzf \
        --prompt="Process> " \
        --header-lines=1 \
        --preview="echo {} | awk '{print \$2}' | xargs ps -fp 2>/dev/null" \
        --preview-window=down:5
fi
SCRIPT

cat > %{buildroot}/opt/freeware/bin/fzf-svc << 'SCRIPT'
#!/opt/freeware/bin/bash
# fzf-svc: Interactive service browser for AIX
# Part of LibrePower (https://librepower.org)
#
# Usage: fzf-svc [filter]
#   Without arguments: interactive browser
#   With argument: filter and show matching services

export PATH=/opt/freeware/bin:$PATH

if [ -n "$1" ]; then
    lssrc -a | fzf --header-lines=1 --filter="$1"
else
    lssrc -a | fzf \
        --prompt="Service> " \
        --header-lines=1
fi
SCRIPT

cat > %{buildroot}/opt/freeware/bin/fzf-hist << 'SCRIPT'
#!/opt/freeware/bin/bash
# fzf-hist: Interactive command history search for AIX
# Part of LibrePower (https://librepower.org)
#
# Usage: fzf-hist [filter]
#   Without arguments: interactive browser
#   With argument: filter and show matching commands

export PATH=/opt/freeware/bin:$PATH

HISTFILE="${HISTFILE:-$HOME/.sh_history}"

if [ ! -f "$HISTFILE" ]; then
    echo "History file not found: $HISTFILE"
    exit 1
fi

if [ -n "$1" ]; then
    cat "$HISTFILE" | fzf --filter="$1" --tac
else
    selected=$(cat "$HISTFILE" | fzf \
        --prompt="History> " \
        --tac \
        --no-sort)
    
    if [ -n "$selected" ]; then
        echo "$selected"
    fi
fi
SCRIPT

chmod 755 %{buildroot}/opt/freeware/bin/fzf-rpm
chmod 755 %{buildroot}/opt/freeware/bin/fzf-proc
chmod 755 %{buildroot}/opt/freeware/bin/fzf-svc
chmod 755 %{buildroot}/opt/freeware/bin/fzf-hist

%files
%defattr(-,root,system,-)
/opt/freeware/bin/fzf
/opt/freeware/bin/fzf-rpm
/opt/freeware/bin/fzf-proc
/opt/freeware/bin/fzf-svc
/opt/freeware/bin/fzf-hist
/opt/freeware/share/fzf/shell/completion.bash
/opt/freeware/share/fzf/shell/completion.zsh
/opt/freeware/share/fzf/shell/key-bindings.bash
/opt/freeware/share/fzf/shell/key-bindings.zsh
/opt/freeware/share/man/man1/fzf.1

%post
echo "========================================================"
echo " fzf 0.46.1 installed successfully!"
echo ""
echo " IMPORTANT - Add to your ~/.bashrc:"
echo "   export FZF_DEFAULT_COMMAND='find . -type f -print 2>/dev/null'"
echo ""
echo " This is required because AIX find differs from GNU find."
echo " Without it, use fzf with piped input: ls | fzf"
echo ""
echo " Quick test:"
echo "   echo 'apple banana orange' | tr ' ' '\n' | fzf"
echo ""
echo " AIX helper scripts included:"
echo "   fzf-rpm   - Browse installed RPM packages"
echo "   fzf-proc  - Browse running processes"
echo "   fzf-svc   - Browse AIX services"
echo "   fzf-hist  - Search command history"
echo ""
echo " For full shell integration, also add to ~/.bashrc:"
echo "   source /opt/freeware/share/fzf/shell/completion.bash"
echo "   source /opt/freeware/share/fzf/shell/key-bindings.bash"
echo ""
echo " SIXE - IBM Business Partner (https://sixe.eu)"
echo " Part of LibrePower (https://librepower.org)"
echo "========================================================"

%changelog
* Thu Jan 02 2025 Hugo Blanco <hugo.blanco@sixe.eu> - 0.46.1-1.aix7.3.sixe
- Initial AIX port for LibrePower
- First Go-based tool successfully compiled for AIX
- Built with Go 1.21.6 official toolchain (not gccgo)
- Includes AIX-specific helper scripts (fzf-rpm, fzf-proc, fzf-svc, fzf-hist)
- Shell integration for bash and zsh
- Note: Requires FZF_DEFAULT_COMMAND for standalone usage
