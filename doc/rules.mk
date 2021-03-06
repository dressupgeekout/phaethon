# Phaethon - A FLOSS resource explorer for BioWare's Aurora engine games
#
# Phaethon is the legal property of its developers, whose names
# can be found in the AUTHORS file distributed with this source
# distribution.
#
# Phaethon is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# Phaethon is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Phaethon. If not, see <http://www.gnu.org/licenses/>.

# Documentation for Phaethon.

# Licenses and credits
dist_doc_DATA += \
    COPYING \
    AUTHORS \
    $(EMPTY)

EXTRA_DIST += \
    doc/LICENSE.GPLv2 \
    doc/LICENSE.LGPLv2.1 \
    $(EMPTY)

# User documentation
dist_doc_DATA += \
    ChangeLog \
    NEWS.md \
    TODO \
    README.md \
    $(EMPTY)

# Documents for contributors
dist_doc_DATA += \
    CONTRIBUTING.md \
    CODE_OF_CONDUCT.md \
    $(EMPTY)
