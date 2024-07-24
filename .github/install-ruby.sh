#!/usr/bin/env bash

# install ruby on CI runners using rbenv, since:
# - using Arch Linux Archive (ALA) for older versions doesn't work (dependency conflicts, etc.)
# - GitHub action ruby/setup-ruby doesn't work for some Linux images

if [ $# -ne 1 ]; then
  echo """
  USAGE: $0 [RUBY_VERSION]
  NOTE: this is for CI usage only
  """
  exit 1
fi

ver=$1

export RBENV_ROOT=$(pwd)/.rbenv
git clone https://github.com/rbenv/rbenv.git $RBENV_ROOT
eval "$(.rbenv/bin/rbenv init - bash)"

git clone https://github.com/rbenv/ruby-build.git $(rbenv root)/plugins/ruby-build
rbenv install $ver
rbenv global $ver
eval "$(rbenv init - bash)"

echo PATH=$PATH >> $GITHUB_ENV
