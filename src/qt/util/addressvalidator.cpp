/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "../LLU/addressvalidator.h"


NexusAddressValidator::NexusAddressValidator(QObject *parent) :
    QValidator(parent)
{
}

QValidator::State NexusAddressValidator::validate(QString &input, int &pos) const
{
    // Correction
    for(int idx=0; idx<input.size();)
    {
        bool removeChar = false;
        QChar ch = input.at(idx);
        // Transform characters that are visually close
        switch(ch.unicode())
        {
        case 'l':
        case 'I':
            input[idx] = QChar('1');
            break;
        case '0':
        case 'O':
            input[idx] = QChar('o');
            break;
        // Qt categorizes these as "Other_Format" not "Separator_Space"
        case 0x200B: // ZERO WIDTH SPACE
        case 0xFEFF: // ZERO WIDTH NO-BREAK SPACE
            removeChar = true;
            break;
        default:
            break;
        }
        // Remove whitespace
        if(ch.isSpace())
            removeChar = true;
        // To next character
        if(removeChar)
            input.remove(idx, 1);
        else
            ++idx;
    }

    // Validation
    QValidator::State state = QValidator::Acceptable;
    for(int idx=0; idx<input.size(); ++idx)
    {
        int ch = input.at(idx).unicode();

        if(((ch >= '0' && ch<='9') ||
           (ch >= 'a' && ch<='z') ||
           (ch >= 'A' && ch<='Z')) &&
           ch != 'l' && ch != 'I' && ch != '0' && ch != 'O')
        {
            // Alphanumeric and not a 'forbidden' character
        }
        else
        {
            state = QValidator::Invalid;
        }
    }

    // Empty address is "intermediate" input
    if(input.isEmpty())
    {
        state = QValidator::Intermediate;
    }

    return state;
}
