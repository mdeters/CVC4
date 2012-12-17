# Example from CAV 2010 paper by David Monniaux, "Quantifier
# Elimination by Lazy Model Enumeration"

# No free vars allowed in this input (though the outermost
# existentials are immediately Skolemized)
Exists[x,
  ForAll[y, Exists[z, And[z>=0, Or[ And[x>=z, y>=z], y<=1-z ] ] ] ]
]
