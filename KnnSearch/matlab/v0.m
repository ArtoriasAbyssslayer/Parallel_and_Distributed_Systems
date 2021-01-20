% todo Generate random 2D X and Y arrays
n = 100 
m = 10
d = 2
k = 5


min = 0
max = 100
randomBounded = (max-min).*rand(1000,1) + min;

X = zeros(n,d); (calloc X)
for i = 1:d
  X(:,i) = (max - min).*rand(n,1) + min;
end

Y = zeros(m,d);
for i = 1:d
  Y(:,i) = (max-min).*rand(m,1) + a;
end

% Calculate D
a = sum(X.^2,2);
b = - 2 * X*Y.';
c = sum(Y.^2,2).';
D = sqrt(sum(X.^2,2) - 2 * X*Y.' + sum(Y.^2,2).');
min_matrix = zeros(k, m);
for col = 1:m
    for i = 1:k
        [min_matrix(i,col), index] = min(D(:,col));
        D(index,col) = inf;
    end
end

