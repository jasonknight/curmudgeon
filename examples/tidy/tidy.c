#include <tidy.h>
#include <string.h>
#include <tidy.h>
#include <buffio.h>
#include <stdio.h>
#include <errno.h>

/**
 * All tidy functions return an integer value that can be
 * interpreted in the following way:
 * 
 * 0 Success, Good to go.
 * 1 Warnings, No Errors. Check error buffer or track error messages for details.
 * 2 Errors and Warnings. By default, Tidy will not produce output. You can
 * force output with the TidyForceOutput option. As with warnings, check error
 * buffer or track error messages for details.
 * <0 Severe error. Usually value equals -errno. See errno.h.
 */
int main(void) {
    char *input = "<title>Foo</title><p>Foo!";
    TidyBuffer output = {0};
    TidyBuffer errbuf = {0};
    int rc = -1;
    Bool ok;

    TidyDoc tdoc = tidyCreate();                     // Initialize "document"
    printf( "Tidying:\t%s\n", input );

    ok = tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  // Convert to XHTML
    if ( ok ) {
        rc = tidySetErrorBuffer( tdoc, &errbuf );      // Capture diagnostics
    }
    
    if ( rc >= 0 ) {
        rc = tidyParseString( tdoc, input );           // Parse the input
    }
    
    if ( rc >= 0 ) {
        rc = tidyCleanAndRepair( tdoc );               // Tidy it up!
    }
    
    if ( rc >= 0 ) {
        rc = tidyRunDiagnostics( tdoc );               // Kvetch
    }
    
    if ( rc > 1 ) {
        rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 ); // If error, force output.
    }
    
    if ( rc >= 0 ) {
        rc = tidySaveBuffer( tdoc, &output );  // Pretty Print
    }

    if ( rc > 0 ) {
        printf( "\nDiagnostics:\n\n%s", errbuf.bp );
    }
    
    if ( rc < 0 ) {
        printf( "A severe error (%d) occurred.\n", rc );
    } else {
        printf( "\nHere is the clean result:\n\n%s", output.bp );
    }

    tidyBufFree( &output );
    tidyBufFree( &errbuf );
    tidyRelease( tdoc );
    return rc;
}