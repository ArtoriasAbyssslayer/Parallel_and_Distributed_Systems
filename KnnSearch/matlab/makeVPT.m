function T = makeVPT(P)
[n,d] = size(P);
T.vp = P(1,:)
if n == 1
    return
else
    d = sqrt(sum(P-T.vp)^2,2);
    T.md = median(d);
    T.int = makeVPT(P(d<T.md,:));
    T.ext = makeVPT(P(d>T.md,:));
    
end
end
