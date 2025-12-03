#include "utils/pdf_report.h"

#include "reports/tiny_pdf.h"

// Implementa um wrapper fino em torno de tiny_pdf.h para expor
// uma função clara para o restante do firmware (e para a Alexa).
void generate_pdf_report() {
    generate_report();
}