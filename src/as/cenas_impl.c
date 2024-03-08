case MNONE:
    if(passno != 1)
    {
        outb(ins->opcode);
    }
    break;
case MREL:
    tok = lex();
    if(passno == 1)
    {
        dotp->xvalue++;
        if (!parseex(&args[0]))
        {
            error("unexp arg");
        }
    }
    else
    {
        if (!parseex(&args[0]))
        {
            error("unexp arg");
        }
        rel = args[0].xvalue - dotp->xvalue;
        printf("line %d: rel %d\n", lineno, rel);
        if (rel > 128 || rel < -127)
        {
            error("out of range");
        }
        outb(ins->opcode);
        outb(rel);
        /*fwrite(&ins->opcode, sizeof(ins->opcode), 1, txtfil);
        fwrite(&rel, 1, 1, txtfil);*/
    }
    break;
case MOD5:
case MOD6:
case MOD14:
    mod = parseargs(m_mod6, MOD6N);
    if (mod == NULL)
    {
        error("bad mode");
    }
    dotp->xvalue += mod->size;
    break;
case MRBRB:
    mod = parseargs(m_rbrb, 1);
    if (mod == NULL)
    {
        error("bad mode");
    }
    dotp->xvalue += mod->size;
    break;
case MRC:
    mod = parseargs(m_rc, 2);
    if (mod == NULL)
    {
        error("bad mode");
    }
    dotp->xvalue += mod->size;
    break;
case MRAIR:
    mod = parseargs(m_rair, 2);
    if (mod == NULL)
    {
        error("bad mode");
    }
    dotp->xvalue += mod->size;
    break;
default:
    error("unexpented");
    break;