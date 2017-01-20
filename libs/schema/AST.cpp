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

#include "AST.hpp"

#include <strtol.h>

#include <klib/symbol.h>
#include <klib/printf.h>
#include <klib/rc.h>

// hide an unfortunately named C function typename()
#define typename __typename
#include "../vdb/schema-parse.h"
#undef typename
#include "../vdb/dbmgr-priv.h"
#include "../vdb/schema-priv.h"
#include "../vdb/schema-expr.h"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

#include "ASTBuilder.hpp"

using namespace std;

// AST

SchemaToken st = { PT_EMPTY, NULL, 0, 0, 0 };

AST :: AST ()
: ParseTree ( st )
{
}

AST :: AST ( Token :: TokenType p_tokenType ) // no-value token
: ParseTree ( Token ( p_tokenType ) )
{
}

AST :: AST ( const Token * p_token )
: ParseTree ( * p_token )
{
}

AST :: AST ( const Token * p_token, AST * p_child1 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
}

AST :: AST ( const Token * p_token, AST * p_child1, AST * p_child2 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
    assert ( p_child2 != 0 );
    AddNode ( p_child2 );
}

AST :: AST ( const Token * p_token, AST * p_child1, AST * p_child2, AST * p_child3 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
    assert ( p_child2 != 0 );
    AddNode ( p_child2 );
    assert ( p_child3 != 0 );
    AddNode ( p_child3 );
}

AST :: AST ( const Token * p_token,
             AST * p_child1,
             AST * p_child2,
             AST * p_child3,
             AST * p_child4,
             AST * p_child5,
             AST * p_child6 )
: ParseTree ( * p_token )
{
    assert ( p_child1 != 0 );
    AddNode ( p_child1 );
    assert ( p_child2 != 0 );
    AddNode ( p_child2 );
    assert ( p_child3 != 0 );
    AddNode ( p_child3 );
    assert ( p_child4 != 0 );
    AddNode ( p_child4 );
    assert ( p_child5 != 0 );
    AddNode ( p_child5 );
    assert ( p_child6 != 0 );
    AddNode ( p_child6 );
}

void
AST :: AddNode ( AST * p_child )
{
    AddChild ( p_child );
}

void
AST :: AddNode ( const Token * p_child )
{
    AddChild ( new AST ( p_child ) );
}

// AST_FQN

AST_FQN :: AST_FQN ( const Token* p_token )
:   AST ( p_token ),
    m_version ( 0 )
{
}

uint32_t
AST_FQN :: NamespaceCount() const
{
    uint32_t count = ChildrenCount ();
    return count > 0 ? ChildrenCount () - 1 : 0;
}

void
AST_FQN :: GetIdentifier ( String & p_str ) const
{
    uint32_t count = ChildrenCount ();
    if ( count > 0 )
    {
        StringInitCString ( & p_str, GetChild ( count - 1 ) -> GetTokenValue () );
    }
    else
    {
        CONST_STRING ( & p_str, "" );
    }
}

void
AST_FQN :: GetFullName ( char* p_buf, size_t p_bufSize ) const
{
    GetPartialName ( p_buf, p_bufSize, ChildrenCount () );
}

void
AST_FQN :: GetPartialName ( char* p_buf, size_t p_bufSize, uint32_t p_lastMember ) const
{
    uint32_t count = ChildrenCount ();
    if ( p_lastMember < count )
    {
        count = p_lastMember + 1;
    }
    size_t offset = 0;
    for ( uint32_t i = 0 ; i < count; ++ i )
    {
        size_t num_writ;
        rc_t rc = string_printf ( p_buf + offset, p_bufSize - offset - 1, & num_writ, "%s%s",
                                  GetChild ( i ) -> GetTokenValue (),
                                  i == count - 1 ? "" : ":" );
        offset += num_writ;
        if ( rc != 0 )
        {
            break;
        }
    }

    p_buf [ p_bufSize ] = 0;
}

void
AST_FQN :: SetVersion ( const char* p_version )
{   // assume the token comes from a scanner which guarantees correctness
    assert ( p_version != 0 );
    assert ( p_version [ 0 ] == '#' );
    const char* str = p_version + 1;
    uint32_t len = string_measure ( str, 0 );
    const char *dot = string_chr ( str, len, '.' );
    m_version = strtou32 ( str, 0, 10 ) << 24;
    if ( dot != 0 )
    {
        str = dot + 1;
        len = string_measure ( str, 0 );
        dot = string_chr ( str, len, '.' );
        m_version |= strtou32 ( str, 0, 10 ) << 16;
        if ( dot != 0 )
        {
            m_version |= strtou32 ( dot + 1, 0, 10 );
        }
    }
}

// AST_Expr

AST_Expr :: AST_Expr ( const Token* p_token )
: AST ( p_token )
{
}

SExpression *
AST_Expr :: EvaluateConst ( ASTBuilder & p_builder ) const
{   //TBD. for now, only handles a literal int constant
    switch ( GetToken () . GetType () )
    {
    case PT_UINT:
        {
            SConstExpr * x = p_builder . Alloc < SConstExpr > ( sizeof * x - sizeof x -> u + sizeof x -> u . u64 [ 0 ] );
            if ( x != 0 )
            {
                assert ( ChildrenCount () == 1 );
                const char * val = GetChild ( 0 ) -> GetTokenValue ();
                uint64_t i64 = 0;
                uint32_t i = 0;
                while ( val [ i ] != 0 )
                {
                    i64 *= 10;
                    i64 += val [ i ] - '0';
                    ++ i;
                }

                x -> u . u64 [ 0 ] = i64;
                x -> dad . var = eConstExpr;
                atomic32_set ( & x -> dad . refcount, 1 );
                x -> td . type_id = p_builder . IntrinsicTypeId ( "U64" );
                x -> td . dim = 1;

                return & x -> dad;
            }
            break;
        }
    default:
        p_builder . ReportError ( "Not yet implemented" );
        break;
    }
    return 0;
}