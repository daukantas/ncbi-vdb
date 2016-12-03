/*===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 */

#ifndef _hpp_SchemaScanner_
#define _hpp_SchemaScanner_

#include "schema-lex.h"

namespace ncbi
{
    namespace SchemaParser
    {
        class SchemaScanner
        {
        public:
            typedef int TokenType;

        public:
            SchemaScanner ( const char * source, bool p_debug = false );
            ~SchemaScanner ();

            :: SchemaScanBlock & GetScanBlock () { return m_scanBlock; }

        public: /* this is for testing only; bison-generated parser calls SchemaScan_yylex directly */
            TokenType Scan();
            const SchemaToken& LastTokenValue() const { return m_lastToken; } // valid until the next call to SchemaScan_yylex

        private:
            :: SchemaScanBlock  m_scanBlock;
            SchemaToken m_lastToken;
        };
    }
}

#endif
