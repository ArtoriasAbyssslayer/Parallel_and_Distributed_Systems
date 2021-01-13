function p=searchVPT(T,P) 
    p = work(T.vp,p)
if ~ (isfield(T,'int') | isfield(T,'ext'))
    return
else
    norm(T.rp-p.coor) < T.md
    p = searchVPT(T.int,p)
    if checkintersection
        p =searchVPT(T.ext,)
end
end