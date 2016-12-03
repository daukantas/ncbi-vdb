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

/**
* Command line test for schema parser.
* Parses the input files and reports errors
*/

#include "../../libs/schema/SchemaParser.hpp"
#include "../../libs/schema/ParseTree.hpp"

using namespace std;
using namespace ncbi::SchemaParser;

//////////////////////////////////////////// Main
#include <kapp/args.h>
#include <kfg/config.h>
#include <klib/out.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

extern "C"
{

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

const char UsageDefaultName[] = "test-schema-parse";

rc_t CC UsageSummary (const char * progname)
{
    return KOutMsg ( "Usage:\n" "\t%s [options] schema-file ... \n\n", progname );
}

rc_t CC Usage( const Args* args )
{
    return 0;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    int failed = 0;
    if ( argc < 2 )
    {
        cout << "Usage:\n\t" << argv[0] << " schema-file" << endl;
        return 1;
    }
    try
    {
        cout << "Parsing " << argc - 1 << " schema-files" << endl;
        for ( int i = 0 ; i < argc - 1; ++i )
        {
            stringstream buffer;
            ifstream in ( argv [ i + 1 ] );
            if ( ! in . good () )
            {
                throw runtime_error ( string ( "Invalid file " ) + argv [ i + 1 ] );
            }
            buffer << in.rdbuf();
            SchemaParser parser;
            if ( ! parser . ParseString ( buffer . str () . c_str () ) )
            {
                cout << string ( "Parsing failed: " ) + argv [ i + 1 ] << endl;
                ++ failed;
            }
        }
        cout << "Failed: " << failed << endl;
    }
    catch ( exception& ex)
    {
        cerr << " Exception: " << ex . what () << endl;
        return 2;
    }
    catch ( ... )
    {
        cerr << " Unknown exception" << endl;
        return 3;
    }
    return failed == 0 ? 0 : 4;
}

}

