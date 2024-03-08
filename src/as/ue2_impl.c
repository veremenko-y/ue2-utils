case MNONE:
    if(passno == 1)
    {
        dotp->xvalue+=2;
    }
    else {
        outb(ins->opcode<<4);
        outb(0);
    }
    break;
case MIMM:
    tok = lex();
    if(passno == 1)
    {
        dotp->xvalue+=2;
    }
    else
    {
        if (!parseex(&args[0]))
        {
            error("unexp arg");
        }
        xtrab = LEN1;
        rel = args[0].xvalue;
        outb(ins->opcode<<4);
        if(args[0].xtype == XABS) {
            /* yaros: todo: add range check [0,255]*/
            TRACE1("0x%04x: ABS %d\n", dotp->xvalue, rel);
        }
        else
        {
            sym = args[0].xname;
            TRACE1("0x%04x: IMMREL %s=%d %d\n", dotp->xvalue, sym->name, sym->value, sym->type);
            /*sym = args[0].xname; unused, debug*/
        }
        outrel(&args[0].xvalue, LEN1, args[0].xtype, args[0].xname);
        /*
        no relative things
        if (rel > int_max || rel < -127)
        {
            error("out of range");
        }*/
    }
    break;
case MABS:
    tok = lex();
    if(passno == 1)
    {
        dotp->xvalue+=2;
       /* if (!parseex(&args[0]))
        {
            error("unexp arg");
        }*/
    }
    else
    {
        if (!parseex(&args[0]))
        {
            error("unexp arg");
        }
        xtrab = LEN2;
        rel = args[0].xvalue;
        sym = args[0].xname;
        TRACE1("rel on line %d = %d\n", lineno, rel);
        TRACE1("0x%04x: ABSREL %s=%d %d\n", dotp->xvalue, sym->name, sym->value, sym->type);
        /*if (rel > 128 || rel < -127)
        {
            error("out of range");
        }*/
        /*outb(ins->opcode);
        outb(rel);*/
        i = (args[0].xvalue & 0xfff) | (ins->opcode << 12);
        outrel(&i, LEN2, args[0].xtype, args[0].xname);
        
        /*fwrite(&ins->opcode, sizeof(ins->opcode), 1, txtfil);
        fwrite(&rel, 1, 1, txtfil);*/
    }
    break;
default:
    error("unexpented");
    break;