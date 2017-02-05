function [J, grad] = linearRegCostFunction(X, y, theta, lambda)

m = length(y); % 训练集的数目

% 下面是需要计算的量
J = 0; % 这里算是事先分配内存
grad = zeros(size(theta));

% X是一个12 * 2的矩阵,而theta是一个2*1类型的矩阵.
h = X * theta; % 得到的结果,是吧.

J = (1 / (2 * m)) * sum((h - y).^2) + (lambda/(2 * m)) *(sum(theta.^2) - theta(1)^2);

% J已经计算出来了,然后需要干的事情就是计算梯度了.
grad = (X' * (h - y))/ m + (lambda / m) * theta; % 然后就开始了.
grad(1) = grad(1) - lambda / m * theta(1);

grad = grad(:); % 展开

end
