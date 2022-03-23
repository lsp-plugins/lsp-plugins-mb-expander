/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins-mb-expander
 * Created on: 3 авг. 2021 г.
 *
 * lsp-plugins-mb-expander is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-plugins-mb-expander is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-plugins-mb-expander. If not, see <https://www.gnu.org/licenses/>.
 */

#include <private/plugins/mb_expander.h>
#include <lsp-plug.in/plug-fw/ui.h>

namespace lsp
{
    namespace plugui
    {
        //---------------------------------------------------------------------
        // Plugin UI factory
        static const meta::plugin_t *plugin_uis[] =
        {
            &meta::mb_expander_mono,
            &meta::mb_expander_stereo,
            &meta::mb_expander_lr,
            &meta::mb_expander_ms,
            &meta::sc_mb_expander_mono,
            &meta::sc_mb_expander_stereo,
            &meta::sc_mb_expander_lr,
            &meta::sc_mb_expander_ms
        };

        static ui::Factory factory(plugin_uis, 8);

    } // namespace plugui
} // namespace lsp


