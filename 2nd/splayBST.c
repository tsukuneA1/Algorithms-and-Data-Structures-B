link
splay (link h, Item item)
{
  Key v = key (item);
  if (h == z)
    return NEW (item, z, z, 1);
  if (less (v, key (h->item)))
    {
      if (h->l == z)
	return NEW (item, z, h, h->N + 1);
      if (less (v, key (h->l->item)))
	{
	  h->l->l = splay (h->l->l, item);
	  h = rotR (h);
	}
      else
	{
	  h->l->r = splay (h->l->r, item);
	  h->l = rotL (h->l);
	}
      return rotR (h);
    }
  else
    {
      if (h->r == z)
	return NEW (item, h, z, h->N + 1);
      if (less (key (h->r->item), v))
	{
	  h->r->r = splay (h->r->r, item);
	  h = rotL (h);
	}
      else
	{
	  h->r->l = splay (h->r->l, item);
	  h->r = rotR (h->r);
	}
      return rotL (h);
    }
}

void
STinsert (Item item)
{
  head = splay (head, item);
}

