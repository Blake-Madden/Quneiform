/********************************************************************************
 * Copyright (c) 2022-2024 Blake Madden
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Blake Madden - initial implementation
 ********************************************************************************/

/** @addtogroup Internationalization
    @brief i18n classes.
@{*/

#ifndef __PO_FILE_REVIEW_H__
#define __PO_FILE_REVIEW_H__

#include "translation_catalog_review.h"
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace i18n_check
    {
    /** @brief Class to extract and review translations from PO catalogs.*/
    class po_file_review : public translation_catalog_review
        {
      public:
        /// @brief Constructor.
        /// @param verbose @c true to include verbose warnings.
        explicit po_file_review(const bool verbose) : translation_catalog_review(verbose) {}

        /** @brief Main interface for extracting resource text from C++ source code.
            @param poFileText The PO file's text to analyze.
            @param fileName The (optional) name of source file being analyzed.*/
        void operator()(std::wstring_view poFileText, const std::filesystem::path& fileName) final;

        /// @brief Whether fuzzy translations should be reviewed.
        /// @param enable @c true to review fuzzy translation entries; @c false to ignore them.
        /// @note It is recommended that this be false, as fuzzy translations are generally
        ///     considered to be "needing work" and are known to have issues.
        void review_fuzzy_translations(const bool enable) noexcept { m_reviewFuzzy = enable; }

        /// @returns @c true if fuzzy translations are being reviewed.\n
        ///     The default is @c false.
        [[nodiscard]]
        bool is_reviewing_fuzzy_translations() const noexcept
            {
            return m_reviewFuzzy;
            }

        /** @brief Finalizes the review process after all files have been loaded.
            @details Reviews the catalogs in all loaded PO files.
            @param resetCallback Callback function to tell the progress system in @c callback
                how many items to expect to be processed.
            @param callback Callback function to display the progress.
                Takes the current file index, overall file count, and the name of the current file.
                Returning @c false indicates that the user cancelled the analysis.*/
        void review_strings(analyze_callback_reset resetCallback, analyze_callback callback) final
            {
            translation_catalog_review::review_strings(resetCallback, callback);
            }

      private:
        bool m_reviewFuzzy{ false };
        };
    } // namespace i18n_check

/** @}*/

#endif // __PO_FILE_REVIEW_H__
